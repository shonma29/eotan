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
#include "../../include/stdarg.h"
#include "../../include/device.h"
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
