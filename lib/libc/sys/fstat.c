/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_fstat.c,v 1.1 1997/08/31 13:25:20 night Exp $  */

#include <errno.h>
#include "posix.h"


/* fstat 
 *
 */
int
fstat (int fd, struct stat *st)
{
  ER			error;
  struct posix_request	req;
  struct posix_response *res = (struct posix_response*)&req;

  req.param.par_fstat.fileid = fd;
  req.param.par_fstat.st = st;

  error = _make_connection(PSC_FSTAT, &req);
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


