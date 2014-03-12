/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_getdents.c,v 1.1 2000/06/01 08:43:45 naniwa Exp $  */

#include "sys.h"


/* getdents
 *
 */
int
getdents (int fd, char *buf, size_t nbytes)
{
  struct posix_request	req;

  req.param.par_getdents.fileid = fd;
  req.param.par_getdents.buf = buf;
  req.param.par_getdents.length = nbytes;

  return _call_fs(PSC_GETDENTS, &req);
}
