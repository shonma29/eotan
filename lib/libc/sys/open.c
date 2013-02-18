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
#include <string.h>
#include "posix.h"


/* open 
 *
 */
int
open (char *path, int oflag, ...)
{
    struct posix_request req;

    req.param.par_open.pathlen = strlen (path);
    req.param.par_open.path = path;
    req.param.par_open.oflag = oflag;	/* o_rdonly | o_wronly | o_rdwr | o_creat */

    if (oflag & O_CREAT) {
      va_list args;

      va_start(args, oflag);
      req.param.par_open.mode = va_arg(args, int);	/* no use except oflag includes o_creat */
    }

    return _call_fs(PSC_OPEN, &req);
}
