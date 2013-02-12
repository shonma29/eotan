/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_dup.c,v 1.1 1997/08/31 13:25:19 night Exp $  */

#include <errno.h>
#include "posix.h"


/* dup 
 *
 */
int
dup (int fd)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_dup.fileid = fd;

  error = _make_connection(PSC_DUP, &req);
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
