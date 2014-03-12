/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_mkdir.c,v 1.1 1997/08/31 13:25:23 night Exp $  */

#include <string.h>
#include "sys.h"


/* mkdir 
 *
 */
int
mkdir (char *path, mode_t mode)
{
  struct posix_request	req;

  req.param.par_mkdir.pathlen = strlen (path);
  req.param.par_mkdir.path = path;
  req.param.par_mkdir.mode = mode;

  return _call_fs(PSC_MKDIR, &req);
}
