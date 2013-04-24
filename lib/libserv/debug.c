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
#include <stdarg.h>
#include <string.h>
#include <itron/types.h>
#include <itron/syscall.h>
#include <mpu/call_kernel.h>
#include "../libserv/libserv.h"

static void dbg_putchar(B ch)
{
    B buf[2];

    buf[0] = ch & 0x000000ff;
    buf[1] = '\0';
    syslog(buf);
}

W dbg_printf(B *format, ...) {
	va_list ap;

	va_start(ap, format);
	return vnprintf(dbg_putchar, format, ap);
}

