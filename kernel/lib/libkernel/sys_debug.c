/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* デバッグ用のシステムコール
 *
 */
#include "../../../include/stdarg.h"
#include "../../core/core.h"
#include "../../core/api.h"

static ER dbg_vprintf(B * fmt, VP arg0);

ER dbg_puts(B * msg)
{
    return call_syscall(SYS_DBG_PUTS, msg);
}


static void dbg_putchar(B ch)
{
    B buf[2];

    buf[0] = ch & 0x000000ff;
    buf[1] = '\0';
    dbg_puts(buf);
}

int dbg_printf(B *format, ...) {
	va_list ap;

	va_start(ap, format);
	return vnprintf(dbg_putchar, format, ap);
}

