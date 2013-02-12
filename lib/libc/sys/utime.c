/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_utime.c,v 1.1 1997/08/31 13:25:28 night Exp $  */

#include <errno.h>
#include <string.h>
#include "posix.h"


/* utime 
 *
 */
int
utime (char *path, struct utimbuf *buf)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_utime.pathlen = strlen (path);
  req.param.par_utime.path = path;
  req.param.par_utime.buf = buf;

  error = _make_connection(PSC_UTIME, &req);
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
