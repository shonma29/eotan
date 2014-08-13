/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_link.c,v 1.1 1997/08/31 13:25:22 night Exp $  */

#include "sys.h"


/* link 
 *
 */
int
link (const char *src, const char *dst)
{
  struct posix_request	req;

  req.param.par_link.src = (char*)src;
  req.param.par_link.dst = (char*)dst;

  return _call_fs(PSC_LINK, &req);
}
