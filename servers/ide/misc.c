/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/misc.c,v 1.2 1997/07/06 11:47:57 night Exp $ */

static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/misc.c,v 1.2 1997/07/06 11:47:57 night Exp $";


/*
 * $Log: misc.c,v $
 * Revision 1.2  1997/07/06 11:47:57  night
 * デバッグ文の引数指定が正しくなかったので修正した。
 *
 * Revision 1.1  1997/05/06 12:43:04  night
 * 最初の登録。
 *
 *
 */

#include "../../include/stdarg.h"
#include "../../lib/libserv/libserv.h"
#include "ide.h"




static ID log_port;
static ID dev_recv;





/* init_log - ログ出力機構を初期化する
 *
 *
 */
void init_log(void)
{
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(DDEV_RES) };

    if (find_port(CONSOLE_DRIVER, &log_port) != E_PORT_OK) {
	dbg_printf("IDE: Cannot open console device.\n");
	ext_tsk();
	/* DO NOT REACHED */
    }

    dev_recv = acre_mbf(&pk_cmbf);

    if (dev_recv <= 0) {
	dbg_printf("IDE: Cannot allocate port\n");
	ext_tsk();
	/* DO NOT REACHED */
    }
}

void putc(B ch)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    W rsize;
    ER error;
    W i;

    req.header.mbfid = dev_recv;
    req.header.msgtyp = DEV_WRI;
    req.body.wri_req.dd = 0xAA;
    req.body.wri_req.size = 1;
    req.body.wri_req.dt[0] = (char) (ch & 0xff);
    error = snd_mbf(log_port, sizeof(req), &req);
    if (error != E_OK) {
	dbg_printf("cannot send packet(%s, %d). %d\n", __FILE__, __LINE__,
		   error);
	return;
    }
    rsize = sizeof(res);
    error = rcv_mbf(&res, (INT *) & rsize, dev_recv);
    if (res.body.wri_res.errcd != E_OK) {
	dbg_printf("%d\n", res.body.wri_res.errcd);
    }
}


W printf(B *format, ...) {
	va_list ap;

	va_start(ap, format);
	return vnprintf(putc, format, ap);
}

/* _assert - ASSERT マクロによって呼び出される関数
 *
 * メッセージを出力し、プログラムを終了する。
 *
 */
void _assert(B * msg)
{
    printf("ASSERT: ");
    printf("%s\n", msg);
    for (;;) {
	ext_tsk();
    }
}
