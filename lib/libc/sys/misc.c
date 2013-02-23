/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_misc.c,v 1.1 2000/01/30 09:06:47 naniwa Exp $
 */

#include "posix.h"

/* misc
 *
 */
int
misc (int cmd, int len, void *argp)
{
  struct posix_request	req;

  req.param.par_misc.cmd = cmd;
  req.param.par_misc.length = len;

  if (cmd == M_SET_PROCINFO) {
    req.param.par_misc.arg.set_procinfo = *((struct procinfo*)argp);
  }
  else {
    req.param.par_misc.arg.procid = (W)argp;
  }

  return _call_fs(PSC_MISC, &req);
}

