/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_dup2.c,v 1.1 1997/08/31 13:25:19 night Exp $  */

#include "sys.h"


/* dup2 
 *
 */
int
dup2 (int fd, int fd2)
{
  struct posix_request	req;

  req.param.par_dup2.fileid1 = fd;
  req.param.par_dup2.fileid2 = fd2;

  return _call_fs(PSC_DUP2, &req);
}
