/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* printk.c --- printk 関係の関数
 *
 */
#include <stdarg.h>
#include <string.h>
#include "core.h"
#include "func.h"
#include "arch/archfunc.h"

/*
 *
 */
W printk(B * fmt,...)
{
    va_list ap;

    va_start(ap, fmt);
    return vnprintf(putchar, fmt, ap);
}

