/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_link.c,v 1.1 1997/08/31 13:25:22 night Exp $  */

#include <string.h>
#include "../native.h"
#include "../errno.h"


/* link 
 *
 */
int
link (char *src, char *dst)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_link.srclen = strlen (src);
  req.param.par_link.src = src;
  req.param.par_link.dstlen = strlen (dst);
  req.param.par_link.dst = dst;

  error = _make_connection(PSC_LINK, &req);
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
