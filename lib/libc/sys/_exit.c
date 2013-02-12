/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_exit.c,v 1.3 2000/02/16 08:14:50 naniwa Exp $  */

#include <errno.h>
#include "posix.h"


/* _exit 
 *
 */
int
_exit (int status)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_exit.evalue = status;
  error = _make_connection(PSC_EXIT, &req);

  /* 自タスクを終了 */
  exd_tsk ();

  /* これ以降は実行されない */
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
