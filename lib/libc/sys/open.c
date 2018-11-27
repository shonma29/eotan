/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_open.c,v 1.3 1999/03/15 08:36:44 monaka Exp $  */

#include <fcntl.h>
#include <stdarg.h>
#include "sys.h"


/* open 
 *
 */
int
open (const char *path, int oflag, ...)
{
    pm_args_t req;

    req.arg1 = (int)path;
    req.arg2 = oflag;	/* o_rdonly | o_wronly | o_rdwr | o_creat */

    if (oflag & O_CREAT) {
      va_list args;

      va_start(args, oflag);
      req.arg3 = va_arg(args, int);	/* no use except oflag includes o_creat */
    }

    return _call_fs(pm_syscall_open, &req);
}
