/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_getdents.c,v 1.1 2000/06/01 08:43:45 naniwa Exp $  */

#include <errno.h>
#include "posix.h"


/* getdents
 *
 */
int
getdents (int fd, char *buf, size_t nbytes)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_getdents.fileid = fd;
  req.param.par_getdents.buf = buf;
  req.param.par_getdents.length = nbytes;

  error = _make_connection(PSC_GETDENTS, &req);
  if (error != E_OK)
    {
      /* What should I do? */
    }

  else if (res->errno)
    {
      errno = res->errno;
      return (-1);
    }

  return (res->status);
}
