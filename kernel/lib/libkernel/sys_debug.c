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

#include "../../ITRON/types.h"
#include "../../ITRON/itron.h"
#include "../../ITRON/syscall.h"
#include "../../ITRON/errno.h"

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


static void print_digit(UW d, UW base)
{
    static B digit_table[] = "0123456789ABCDEF";

    if (d < base) {
	dbg_putchar(digit_table[d]);
    } else {
	print_digit(d / base, base);
	dbg_putchar(digit_table[d % base]);
    }
}


static void print_string(B * string)
{
    dbg_puts(string);
}

#ifdef notdef
#define INC(p,x)	(((W)p) = (((W)p) + sizeof (x *)))
#endif

#define INC(p,x)	(p = (VP)(((W)p) + sizeof (x)))

/*
 *
 */
W dbg_printf(B * fmt,...)
{
    VP arg0;

    arg0 = (VP) & fmt;
    INC(arg0, B *);
    return (dbg_vprintf(fmt, (VP) arg0));
}

static ER dbg_vprintf(B * fmt, VP arg0)
{
    VP *ap;

    for (ap = (VP *) arg0; *fmt != '\0'; fmt++) {
	if (*fmt == '%') {
	    switch (*++fmt) {
	    case 's':
		print_string((B *) (*ap));
		INC(ap, B *);
		break;

	    case 'd':
		if ((W) * ap < 0) {
		    W *q = (W*)ap;

		    *q = -((W) * ap);
		    dbg_putchar('-');
		}
		print_digit((W) * ap, 10);
		INC(ap, W);
		break;

	    case 'x':
		print_digit((UW) * ap, 16);
		INC(ap, W);
		break;

	    default:
		dbg_putchar('%');
		break;
	    }
	} else {
	    dbg_putchar(*fmt);
	}
    }
}
