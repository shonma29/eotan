/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_umask.c,v 1.1 1997/08/31 13:25:27 night Exp $  */

#include "posix.h"


/* umask 
 *
 */
mode_t
umask (mode_t mask)
{
  struct posix_request	req;

  req.param.par_umask.umask = mask;

  return _call_fs(PSC_UMASK, &req);
}

