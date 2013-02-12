/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_lseek.c,v 1.1 1997/08/31 13:25:23 night Exp $  */

#include "../native.h"
#include "../errno.h"


/* lseek 
 *
 */
int
lseek (int fd, int offset, int mode)
{
  ER			error;
  struct posix_request	req;
  struct posix_response *res = (struct posix_response*)&req;

  req.param.par_lseek.fileid = fd;
  req.param.par_lseek.offset = offset;
  req.param.par_lseek.mode = mode;

  error = _make_connection(PSC_LSEEK, &req);
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


