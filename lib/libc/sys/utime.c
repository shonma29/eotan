/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_utime.c,v 1.1 1997/08/31 13:25:28 night Exp $  */

#include <string.h>
#include "sys.h"


/* utime 
 *
 */
int
utime (char *path, struct utimbuf *buf)
{
  struct posix_request	req;

  req.param.par_utime.pathlen = strlen (path);
  req.param.par_utime.path = path;
  req.param.par_utime.buf = buf;

  return _call_fs(PSC_UTIME, &req);
}
