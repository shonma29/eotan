/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_fstat.c,v 1.1 1997/08/31 13:25:20 night Exp $  */

#include "sys.h"


/* fstat 
 *
 */
int
fstat (int fd, struct stat *st)
{
  struct posix_request	req;

  req.param.par_fstat.fileid = fd;
  req.param.par_fstat.st = st;

  return _call_fs(PSC_FSTAT, &req);
}


