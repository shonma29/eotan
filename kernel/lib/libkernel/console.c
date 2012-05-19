/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/console.c,v 1.2 1999/12/18 15:55:01 kishida0 Exp $ */
static char rcsid[] = "@(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/console.c,v 1.2 1999/12/18 15:55:01 kishida0 Exp $";

/*
 * $Log: console.c,v $
 * Revision 1.2  1999/12/18 15:55:01  kishida0
 * $B%^%$%J!<$J=$@5(B
 *
 * Revision 1.1  1999/04/18 17:48:33  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.1  1996/07/22 23:52:05  night
 * 最初の登録
 *
 *
 */

#include <itron.h>
#include "device.h"
#include "port-manager.h"

#define CONSOLE_DRIVER	"driver.console"


/* コンソールデバイスドライバに対してメッセージを出力するための関数群 
 *
 */

/* ----------------------------------------------------------- */
/*                  ファイル内 static 変数                     */
/* ----------------------------------------------------------- */
 
static ID	console;
static ID	recv;


/* open_console
 *
 * コンソールデバイスドライバをオープンする。
 *
 * 引数		なし
 *
 * 返り値	エラー番号
 *
 */
ER
open_console (void)
{
  recv = get_port (sizeof (DDEV_RES), sizeof (DDEV_RES));
  if (recv <= 0)
    {
      return (E_SYS);
    }

  if (find_port (CONSOLE_DRIVER, &console) != E_PORT_OK)
    {
      return (E_SYS);
    }
  
  return (E_OK);
}


/* write_console
 *
 * コンソールへの文字出力
 *
 *
 */
ER
write_console (B *buf, W length)
{
  DDEV_REQ		req;		/* 要求パケット */
  DDEV_RES		res;		/* 返答パケット */
  W			rsize;
  ER			error;
  W			i;
  
  req.header.mbfid = recv;
  req.header.msgtyp = DEV_WRI;
  req.body.wri_req.size = length;
  bcopy (buf, req.body.wri_req.dt, length);
  error = snd_mbf (console, sizeof (req), &req);
  if (error != E_OK)
    {
      return (error);
    }
  rsize = sizeof (res);
  error = rcv_mbf (&res, (INT *)&rsize, recv);
  if (res.body.wri_res.errcd != E_OK)
    {
      return (res.body.wri_res.errcd);
    }      
  return (E_OK);
}
