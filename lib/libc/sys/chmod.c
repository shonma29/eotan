/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_chmod.c,v 1.1 1997/08/31 13:25:19 night Exp $  */

#include "sys.h"


/* chmod 
 *
 */
int
chmod (char *path, mode_t mode)
{
  struct posix_request	req;

  req.param.par_chmod.path = path;
  req.param.par_chmod.mode = mode;

  return _call_fs(PSC_CHMOD, &req);
}


