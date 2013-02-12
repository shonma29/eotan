/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_kill.c,v 1.1 1997/08/31 13:25:22 night Exp $  */

#include <errno.h>
#include "posix.h"


/* kill 
 *
 */
int
kill (int pid)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_kill.pid = pid;

  error = _make_connection(PSC_KILL, &req);
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
