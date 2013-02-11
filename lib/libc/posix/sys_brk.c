/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header$  */

#include "../native.h"
#include "../errno.h"


/* brk
 *
 */
int
brk (void *endds)
{
  ER			error;
  struct posix_request	req;
  struct posix_response *res = (struct posix_response*)&req;

  req.param.par_brk.end_adr = endds;

  error = _make_connection(PSC_BRK, &req);
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
