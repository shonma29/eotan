/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* error.cj
 *
 */
#include "sync.h"
#include "mpu/mpufunc.h"


/* falldown --- ITRON での [panic] 関数。
 *
 * @desc
 *	falldown() は、引数で指定されたフォーマットに従い、(コンソール)画面に
 *	メッセージを表示する。
 *	フォーマットの意味は、printk () と同じ。
 *
 *	メッセージを表示した後は、無限ループに入る。
 *
 */
void falldown()
{
    enter_critical();
    halt();
}
