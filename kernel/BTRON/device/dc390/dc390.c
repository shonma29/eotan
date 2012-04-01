/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/dc390/dc390.c,v 1.2 1999/06/20 14:46:41 kishida0 Exp $*/
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/dc390/dc390.c,v 1.2 1999/06/20 14:46:41 kishida0 Exp $";


/* 
 * $Log: dc390.c,v $
 * Revision 1.2  1999/06/20 14:46:41  kishida0
 * write defines for debug.
 *
 * Revision 1.1  1999/04/26 18:33:32  kishida0
 * add for debug
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */

/*
 *
 *
 */

#include "dc390.h"
#include "dc390_internal.h"


/*
 *	 局所変数群の宣言
 *
 */
static W	mydevid;	/* 自分自身のid */
static ID	recvport;	/* 要求受けつけ用ポート */
static W	initialized;

struct device	dev_table[MAX_DC390];


static void	main_loop (void);
static void	init_driver (void);


/* start --- dc390 ドライバのメイン関数
 */
void
start ( void )
{
  probe (&dev_table[0]); /* デバイスが存在しているかをチェックする */
  init_driver ();

  printf ("registed dc390 driver.\n");
  main_loop ();
}


/*
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

  init_log ();

  /*
   * 要求受けつけ用のポートを初期化する。
   */
  recvport = get_port (sizeof (DDEV_RES), sizeof (DDEV_RES));
  if (recvport <= 0)
    {
      dbg_printf ("dc390: cannot make receive porrt.\n");
      slp_tsk ();
      /* メッセージバッファ生成に失敗 */
    }

  error = regist_port (DC390_DRIVER, recvport);
  if (error != E_OK)
    {
      /* error */
    }

}



/*
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

	default:
	  /* Unknown error */
	  slp_tsk ();
	  break;
	}
    }

  /* ここの行には、来ない */
}


/*
 *
 */
ER
process_request (DDEV_REQ *req)
{
  switch (req->header.msgtyp)
    {
    case DEV_OPN:
      /* デバイスのオープン */
      open_dc390 (req->header.mbfid, &(req->body.opn_req));
      break;

    case DEV_CLS:
      /* デバイスのクローズ */
      close_dc390 (req->header.mbfid, &(req->body.cls_req));
      break;

    case DEV_REA:
      read_dc390 (req->header.mbfid, &(req->body.rea_req));
      break;

    case DEV_WRI:
      write_dc390 (req->header.mbfid, &(req->body.wri_req));
      break;

    case DEV_CTL:
      control_dc390 (req->header.mbfid, &(req->body.ctl_req));
      break;
    }
}

/*
 * デバイスのオープン
 */
ER
open_dc390 (ID caller, DDEV_OPN_REQ *packet)
{
UH VenderID,DeviceID,Command,Status,
  // PCIC OPEN 
  outb(0xF1, 0xCF8);
  outb(0x00, 0xCFA);

  GetPCI-ConigurationSpaceHeader(&pci-inf);
  // Get Vendor ID
  VenderID = inw(0x00);
  



  return (E_NOSPT);
}

/*
 * デバイスのクローズ
 */
ER
close_dc390 (ID caller, DDEV_CLS_REQ *packet)
{
  /*** ここにコードをついかしてください ***/

  return (E_NOSPT);
}

/*
 *
 */
ER
read_dc390 (ID caller, DDEV_REA_REQ *packet)
{
  /*** ここにコードをついかしてください ***/

  return (E_NOSPT);
}

/*
 *
 */
ER
write_dc390 (ID caller, DDEV_WRI_REQ *packet)
{
  /*** ここにコードをついかしてください ***/

  return (E_NOSPT);
}

/*
 *
 */
ER
control_dc390 (ID caller, DDEV_CTL_REQ *packet)
{
  /*** ここにコードをついかしてください ***/

  return (E_NOSPT);
}
