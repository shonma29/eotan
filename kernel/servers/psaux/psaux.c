/*
   psaux.h

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Modified by Tomohide Naniwa, 2001
*/
#include "../../../include/device.h"
#include "../../../include/mpu/io.h"
#include "../../lib/libkernel/libkernel.h"
#include "psaux.h"
#include "kbc.h"

W       mydevid;	   /* 自分自身のid */
ID      recvport;	   /* 要求受けつけ用ポート */
ID      waitflag;          /* 入力待ちフラグ　*/          

W       driver_opened;     /* ドライバオープン時の排他制御用 */
W       driver_mode;       /* ドライバモード */ 

/*********************************************************************
 * 初期化
 *
 * o ファイルテーブル (file_table) の初期化
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */

static void
init_driver (void)
{
  int		i;
  ER		error;

  /*
   * 要求受けつけ用のポートを初期化する。
   */
  recvport = get_port (0, sizeof (DDEV_RES));

  if (recvport <= 0)
    {
      dbg_printf ("psaux: cannot make receive port.\n");
      slp_tsk ();
      /* メッセージバッファ生成に失敗 */
    }

  error = regist_port (PSAUX_DRIVER, recvport);
  if (error != E_OK)
    {
      /* error */
      dbg_printf("psaux: cannot make receive port.\n"); 
      slp_tsk();
    }

  /* イベントフラグを生成 */
  waitflag = get_flag (TA_WSGL, 0);   

  /* バッファの初期化 */
  init_buffer();

  /* 割り込みハンドラの登録  */
  init_interrupt();
  reset_intr_mask(PSAUX_IRQ); 

  /* キーボードコントローラ初期化 */
  dis_int();
  kbc_wr_command(0x60);  
  kbc_wr_data(0x61);     
  ena_int();

  driver_mode = NOWAITMODE;
  driver_opened = FALSE;
}

/*********************************************************************
 *
 */
static void
main_loop (void)
{
  DDEV_REQ	req;
  extern ER	sys_errno;
  UW		rsize;

  /*
   * 要求受信 - 処理のループ
   */
  for (;;)
    {
      /* 要求の受信 */
      get_req (recvport, &req, &rsize);
      switch (sys_errno)
	{
	case E_OK:
	  /* 正常ケース */
	  process_request (&req);
	  break;

	case E_TMOUT:
	case E_RLWAI:
	  break;
	  
	default:
	  /* Unknown error */
	  dbg_printf ("PSAUX: get_req() Unknown error");
	  break;
	}
    }

  /* ここの行には、来ない */
}


/*********************************************************************
 *
 */
ER
process_request (DDEV_REQ *req)
{
  switch (req->header.msgtyp)
    {
    case DEV_OPN:
      /* デバイスのオープン */
      open_psaux (req->header.mbfid, &(req->body.opn_req));
      break;

    case DEV_CLS:
      /* デバイスのクローズ */
      close_psaux (req->header.mbfid, &(req->body.cls_req));
      break;

    case DEV_REA:
      read_psaux (req->header.mbfid, &(req->body.rea_req));
      break;

    case DEV_PRD:
      posix_read_psaux (req->header.mbfid, req->header.tskid,
			   &(req->body.prd_req));
      break;

    case DEV_WRI:
      write_psaux (req->header.mbfid, &(req->body.wri_req));
      break;

    case DEV_CTL:
      control_psaux (req->header.mbfid, &(req->body.ctl_req));
      break;
    }
}


/********************************************************************* 
 * デバイスドライバ登録時に最初に呼び出される
 *
 */

void
start (void)
{
  init_driver ();
  dbg_printf ("registed psaux driver\n");
  main_loop ();
}


/*********************************************************************
 *
 *
 */

ER
open_psaux (ID caller, DDEV_OPN_REQ *packet)
{
  DDEV_RES res;

  driver_mode = NOWAITMODE;

  res.body.opn_res.dd = packet->dd;
  res.body.opn_res.size = 0;
 
  if(driver_opened == TRUE)
    {
      res.body.opn_res.errcd = E_DEV; 
      res.body.opn_res.errinfo = E_DEV;	
    }
  else
    {
      res.body.opn_res.errcd = E_OK;
      res.body.opn_res.errinfo = E_OK;

      driver_opened = TRUE;

      /* PSAUX device enable */
      dis_int();   
      init_buffer();
      kbc_wr_command(0xA8); /* aux interface enable */
      kbc_wr_command(0x60);
#ifdef notdef
      kbc_wr_data(0x43);
#else
      kbc_wr_data(0x47);
#endif
      ena_int();
    }

  snd_mbf(caller, sizeof(res), &res);
  return E_OK;
}

/*
 *
 */
ER
close_psaux (ID caller, DDEV_CLS_REQ *packet)
{
  DDEV_RES res;

  driver_opened = FALSE;

  /* PSAUX device disable */
  dis_int();            
  kbc_wr_command(0x60);
#ifdef notdef
  kbc_wr_data(0x61);
#else
  kbc_wr_data(0x65);
#endif
  kbc_wr_command(0xA7); /* aux interface disable */
  ena_int();

  res.body.cls_res.dd = packet->dd;
  res.body.cls_res.errcd = E_OK;
  res.body.cls_res.errinfo = E_OK;
  snd_mbf(caller, sizeof(res), &res);
  return E_OK;
}

/*
 *
 */
ER
read_psaux (ID caller, DDEV_REA_REQ *packet)
{
  DDEV_RES	res;
  W		i;
  B             data;

  res.body.rea_res.dd = packet->dd;
  for (i = 0; i < packet->size; i++)
    {
      if(psaux_data_in(&data) == FALSE)
	break;
      res.body.rea_res.dt[i] = data;
    }
  res.body.rea_res.a_size = i;
  res.body.rea_res.errcd = E_OK;
  res.body.rea_res.errinfo = E_OK;
  snd_mbf (caller, sizeof (res), &res);
  return (E_OK);  
}

/*
 *
 */
ER
posix_read_psaux (ID caller, ID tskid, DDEV_PRD_REQ *packet)
{
  struct posix_response res;
  W      i;
  B      data;
  B      buf[packet->length+1];

  for (i = 0; i < packet->length; i++) 
    {
      if(psaux_data_in(&data) == FALSE)
	break;
      buf[i] = data;
    }
  buf[packet->length] = 0;
  vput_reg(tskid, packet->buf, packet->length, buf);
  
  res.receive_port = 0;
  res.msg_length = sizeof (res);
  res.operation = PSC_READ;
  res.errno = EP_OK;
  res.status = i; 
  res.ret1 = 0;
  res.ret2 = 0;
  snd_mbf (caller, sizeof (res), &res);
  return (E_OK);
}

/*
 *
 */
ER
write_psaux (ID caller, DDEV_WRI_REQ *packet)
{
  DDEV_RES	res;
  H             i;
  B data;

  for(i = 0; i < packet->size; i++)
    {
      psaux_data_out(packet->dt[i]);
    }
  res.body.rea_res.dd = packet->dd;
  res.body.rea_res.errcd = E_OK;
  res.body.rea_res.errinfo = E_OK;
  snd_mbf (caller, sizeof (res), &res);
  return (E_OK);
}

/*
 *
 */
ER
control_psaux (ID caller, DDEV_CTL_REQ *packet)
{
  DDEV_RES      res;

  switch (packet->cmd)
    {
    case PSAUX_CLEAR:
      dis_int();
      init_buffer();
      ena_int();
      res.body.ctl_res.dd = packet->dd;
      res.body.ctl_res.errcd = E_OK;
      res.body.ctl_res.errinfo = E_OK;
      snd_mbf (caller, sizeof (res), &res);
      return (E_OK);
      break;

    case PSAUX_CHANGEMODE:
      driver_mode = packet->param[0];
      res.body.ctl_res.dd = packet->dd;
      res.body.ctl_res.errcd = E_OK;
      res.body.ctl_res.errinfo = E_OK;
      snd_mbf (caller, sizeof (res), &res);
      return (E_OK);
      break;

    default:
      res.body.ctl_res.dd = packet->dd;
      res.body.ctl_res.errcd = E_NOSPT;
      res.body.ctl_res.errinfo = E_NOSPT;
      snd_mbf (caller, sizeof (res), &res);
      return (E_NOSPT);
    }
}



