/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_geteuid.c,v 1.1 1997/08/31 13:25:21 night Exp $  */

#include "sys.h"


/* geteuid 
 *
 */
int
geteuid (void)
{
  struct posix_request	req;

  return _call_fs(PSC_GETEUID, &req);
}


