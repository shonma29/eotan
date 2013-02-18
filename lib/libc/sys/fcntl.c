/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_fcntl.c,v 1.2 2000/01/29 16:23:08 naniwa Exp $  */

#include <stdarg.h>
#include "posix.h"


/* fcntl 
 *
 */
int
fcntl (int fileid, int cmd, ...)
{
  struct posix_request	req;
  va_list args;

  va_start(args, cmd);

  req.param.par_fcntl.fileid = fileid;
  req.param.par_fcntl.cmd = cmd;
  req.param.par_fcntl.arg = va_arg(args, void*);

  return _call_fs(PSC_FCNTL, &req);
}
