/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_rmdir.c,v 1.1 1997/08/31 13:25:24 night Exp $  */

#include <string.h>
#include "posix.h"


/* rmdir 
 *
 */
int
rmdir (const char *path)
{
  struct posix_request	req;

  req.param.par_rmdir.pathlen = strlen (path);
  req.param.par_rmdir.path = (char*)path;

  return _call_fs(PSC_RMDIR, &req);
}
