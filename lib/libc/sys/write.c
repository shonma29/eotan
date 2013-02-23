/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_write.c,v 1.1 1997/08/31 13:25:28 night Exp $  */

#include "posix.h"


/* write 
 *
 */
int
write (int fd, void *buf, int length)
{
  struct posix_request	req;

  req.param.par_write.fileid = fd;
  req.param.par_write.buf = buf;
  req.param.par_write.length = length;

  return _call_fs(PSC_WRITE, &req);
}

