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
#include <device.h>
#include <itron/rendezvous.h>
#include <mpu/io.h>
#include <arch/archfunc.h>
#include <arch/8259a.h>
#include "../../lib/libserv/libserv.h"
#include "../../lib/libserv/port.h"
#include "psaux.h"
#include "kbc.h"

W       mydevid;	   /* 自分自身のid */
ID      recvport;	   /* 要求受けつけ用ポート */
ID      waitflag;          /* 入力待ちフラグ　*/          

W       driver_opened;     /* ドライバオープン時の排他制御用 */
W       driver_mode;       /* ドライバモード */ 

static ER process_request (RDVNO rdvno, devmsg_t *packet);
static ER open_psaux (RDVNO rdvno, devmsg_t *packet);
static ER close_psaux (RDVNO rdvno, devmsg_t *packet);
static ER read_psaux (RDVNO rdvno, devmsg_t *packet);
static ER write_psaux (RDVNO rdvno, devmsg_t *packet);
static ER control_psaux (RDVNO rdvno, devmsg_t *packet);

/*********************************************************************
 * 初期化
 *
 * o ファイルテーブル (file_table) の初期化
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */

static void
init_driver (void)
{
  ER		error;
  T_CPOR pk_cpor = { TA_TFIFO, sizeof(DDEV_REQ), sizeof(DDEV_RES) };
  T_CFLG pk_cflg = { NULL, TA_WSGL, 0 };

  /*
   * 要求受けつけ用のポートを初期化する。
   */
  recvport = acre_por(&pk_cpor);

  if (recvport <= 0)
    {
      dbg_printf ("[PSAUX] acre_por error = %d\n", recvport);
      ext_tsk ();
      /* メッセージバッファ生成に失敗 */
    }

  error = regist_port ((port_name*)PSAUX_DRIVER, recvport);
  if (error != E_OK)
    {
      /* error */
      dbg_printf("[PSAUX] cannot make receive port. error = %d\n", error); 
      ext_tsk();
    }

  /* イベントフラグを生成 */
  waitflag = acre_flg(&pk_cflg);   

  /* バッファの初期化 */
  init_buffer();

  /* 割り込みハンドラの登録  */
  psaux_interrupt_initialize();
  pic_reset_mask(ir_mouse); 

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
  /*
   * 要求受信 - 処理のループ
   */
  for (;;)
    {
      devmsg_t packet;
      ER_UINT rsize;
      RDVNO rdvno;

      /* 要求の受信 */
      rsize = acp_por(recvport, 0xffffffff, &rdvno, &packet);
      if (rsize >= 0)
	{
	  /* 正常ケース */
	  process_request (rdvno, &packet);
	}
	else
	{
	  /* Unknown error */
	  dbg_printf ("[PSAUX] acp_por error = %d\n", rsize);
	}
    }

  /* ここの行には、来ない */
}


/*********************************************************************
 *
 */
static ER
process_request (RDVNO rdvno, devmsg_t *packet)
{
  ER result = E_NOSPT;

  switch (packet->req.header.msgtyp)
    {
    case DEV_OPN:
      /* デバイスのオープン */
      result = open_psaux (rdvno, packet);
      break;

    case DEV_CLS:
      /* デバイスのクローズ */
      result = close_psaux (rdvno, packet);
      break;

    case DEV_REA:
      result = read_psaux (rdvno, packet);
      break;

    case DEV_WRI:
      result = write_psaux (rdvno, packet);
      break;

    case DEV_CTL:
      result = control_psaux (rdvno, packet);
      break;
    }

    return result;
}


/********************************************************************* 
 * デバイスドライバ登録時に最初に呼び出される
 *
 */

void
start (void)
{
  init_driver ();
  dbg_printf ("[PSAUX] started. port = %d\n", recvport);
  main_loop ();
}


/*********************************************************************
 *
 *
 */

static ER
open_psaux (RDVNO rdvno, devmsg_t *packet)
{
  DDEV_OPN_REQ * req = &(packet->req.body.opn_req);
  DDEV_OPN_RES * res = &(packet->res.body.opn_res);

  driver_mode = NOWAITMODE;

  res->dd = req->dd;
  res->size = 0;
 
  if(driver_opened == TRUE)
    {
      res->errcd = E_DEV; 
      res->errinfo = E_DEV;	
    }
  else
    {
      res->errcd = E_OK;
      res->errinfo = E_OK;

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

  rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
  return E_OK;
}

/*
 *
 */
static ER
close_psaux (RDVNO rdvno, devmsg_t *packet)
{
  DDEV_CLS_REQ * req = &(packet->req.body.cls_req);
  DDEV_CLS_RES * res = &(packet->res.body.cls_res);

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

  res->dd = req->dd;
  res->errcd = E_OK;
  res->errinfo = E_OK;
  rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
  return E_OK;
}

/*
 *
 */
static ER
read_psaux (RDVNO rdvno, devmsg_t *packet)
{
  DDEV_REA_REQ * req = &(packet->req.body.rea_req);
  DDEV_REA_RES * res = &(packet->res.body.rea_res);
  W		i;
  B             data;

  res->dd = req->dd;
  for (i = 0; i < req->size; i++)
    {
      if(psaux_data_in(&data) == FALSE)
	break;
      res->dt[i] = data;
    }
  res->a_size = i;
  res->errcd = E_OK;
  res->errinfo = E_OK;
  rpl_rdv (rdvno, packet, sizeof (DDEV_RES));
  return (E_OK);  
}

/*
 *
 */
static ER
write_psaux (RDVNO rdvno, devmsg_t *packet)
{
  DDEV_WRI_REQ * req = &(packet->req.body.wri_req);
  DDEV_WRI_RES * res = &(packet->res.body.wri_res);
  H             i;

  for(i = 0; i < req->size; i++)
    {
      psaux_data_out(req->dt[i]);
    }
  res->dd = req->dd;
  res->errcd = E_OK;
  res->errinfo = E_OK;
  rpl_rdv (rdvno, packet, sizeof (DDEV_RES));
  return (E_OK);
}

/*
 *
 */
static ER
control_psaux (RDVNO rdvno, devmsg_t *packet)
{
  DDEV_CTL_REQ * req = &(packet->req.body.ctl_req);
  DDEV_CTL_RES * res = &(packet->res.body.ctl_res);

  switch (req->cmd)
    {
    case PSAUX_CLEAR:
      dis_int();
      init_buffer();
      ena_int();
      res->dd = req->dd;
      res->errcd = E_OK;
      res->errinfo = E_OK;
      rpl_rdv (rdvno, packet, sizeof (DDEV_RES));
      return (E_OK);
      break;

    case PSAUX_CHANGEMODE:
      driver_mode = req->param[0];
      res->dd = req->dd;
      res->errcd = E_OK;
      res->errinfo = E_OK;
      rpl_rdv (rdvno, packet, sizeof (DDEV_RES));
      return (E_OK);
      break;

    default:
      res->dd = req->dd;
      res->errcd = E_NOSPT;
      res->errinfo = E_NOSPT;
      rpl_rdv (rdvno, packet, sizeof (DDEV_RES));
      return (E_NOSPT);
    }
}
