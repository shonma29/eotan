/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* log.c - ログ出力用の関数
 *
 *
 *
 * $Log: log.c,v $
 * Revision 1.8  1999/03/24 03:54:45  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.7  1997/10/24 13:56:58  night
 * 必要な return 文を追加。
 *
 * Revision 1.6  1997/08/31 13:30:09  night
 * 文字列出力時につけていた引用符の出力を削除した。
 * (本当の出力とまぎらわしいため)
 *
 * Revision 1.5  1997/05/08 15:11:29  night
 * プロセスの情報を設定する機能の追加。
 * (syscall misc の proc_set_info コマンド)
 *
 * Revision 1.4  1997/04/28 15:28:04  night
 * インクリメントサイズの計算に sizeof(type *) とやっていたところを
 * sizeof(type) に変更した。
 *
 * Revision 1.3  1997/04/24 15:40:30  night
 * mountroot システムコールの実装を行った。
 *
 * Revision 1.2  1996/11/11  13:36:54  night
 * Assert 実行時の表示を一部変更
 *
 * Revision 1.1  1996/11/05  15:13:46  night
 * 最初の登録
 *
 */

#include "posix.h"

static ID log_port;
static ID dev_recv;

static ER putc(B ch);




/* init_log - ログ出力機構を初期化する
 *
 *
 */
void init_log(void)
{
    if (find_port(CONSOLE_DRIVER, &log_port) != E_PORT_OK) {
	dbg_printf("POSIX: Cannot open console device.\n");
	slp_tsk();
	/* DO NOT REACHED */
    }
#ifdef notdef
    dev_recv = get_port(sizeof(DDEV_RES), sizeof(DDEV_RES));
#else
    dev_recv = get_port(0, sizeof(DDEV_RES));
#endif
    if (dev_recv <= 0) {
	dbg_printf("POSIX: Cannot allocate port\n");
	slp_tsk();
	/* DO NOT REACHED */
    }
}

/*
 *
 */
W printk(B * fmt,...)
{
    va_list ap;

    va_start(ap, fmt);
    return vnprintf(putc, fmt, ap);
}

static ER putc(B ch)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    W rsize;
    ER error;
    ID new_port;

    req.header.mbfid = dev_recv;
    req.header.msgtyp = DEV_WRI;
    req.body.wri_req.dd = 0x00;
    req.body.wri_req.size = 1;
    req.body.wri_req.dt[0] = (char) (ch & 0xff);
    error = snd_mbf(log_port, sizeof(req), &req);
    if (error != E_OK) {
	dbg_printf("cannot send packet. %d\n", error);
	return (EP_IO);
    }
    rsize = sizeof(res);
    error = rcv_mbf(&res, (INT *) & rsize, dev_recv);
    if (res.body.wri_res.errcd != E_OK) {
	dbg_printf("%d\n", res.body.wri_res.errcd);
	return (EP_IO);
    }
    return (EP_OK);
}




/* _assert - ASSERT マクロによって呼び出される関数
 *
 * メッセージを出力し、プログラムを終了する。
 *
 */
void _assert(B * msg)
{
    printk("ASSERT: ");
    printk("%s\n", msg);
    for (;;) {
	slp_tsk();
    }
}
