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
#include <core.h>
#include "sys.h"


/* _exit 
 *
 */
void
_exit (int status)
{
  struct posix_request	req;

  req.param.par_exit.evalue = status;
  _make_connection(PSC_EXIT, &req);
  for (;;);
}
