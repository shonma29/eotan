/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header$  */

#include "posix.h"


/* brk
 *
 */
int
brk (void *endds)
{
  struct posix_request	req;

  req.param.par_brk.end_adr = endds;

  return _call_fs(PSC_BRK, &req);
}
