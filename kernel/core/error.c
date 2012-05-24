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
#include "../../include/stdarg.h"
#include "../../include/mpu/io.h"
#include "core.h"
#include "version.h"
#include "func.h"


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
void falldown(B * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vnprintf(putchar, fmt, ap);

/* もし、ITRON_DBG マクロが定義されていれば、デバッガを起動する。
 */
#ifdef ITRON_DBG
    itron_debugger();
#else
    dis_int();
#ifndef I386
    for (;;);
#else
    asm("hlt");
#endif
#endif				/* ITRON_DBG */
}
