/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_setgid.c,v 1.1 1997/08/31 13:25:24 night Exp $  */

#include "sys.h"


/* setgid 
 *
 */
int
setgid (int gid)
{
  struct posix_request	req;

  req.param.par_setgid.gid = gid;

  return _call_fs(PSC_SETGID, &req);
}


