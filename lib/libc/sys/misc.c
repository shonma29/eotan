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

#include "../native.h"
#include "../errno.h"

/* misc
 *
 */
int
misc (int cmd, int len, void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_misc.cmd = cmd;
  req.param.par_misc.length = len;

  if (cmd == M_SET_PROCINFO) {
    req.param.par_misc.arg.set_procinfo = *((struct procinfo*)argp);
  }
  else {
    req.param.par_misc.arg.procid = (W)argp;
  }

  error = _make_connection(PSC_MISC, &req);
  if (error != E_OK)
    {
      /* What should I do? */
    }

  else if (res->errno)
    {
      errno = res->errno;
      return (-1);
    }

  return (res->status);
}


