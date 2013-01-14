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
#include "../native.h"


/* fcntl 
 *
 */
int
fcntl (int fileid, int cmd, ...)
{
  va_list args;

  va_start(args, cmd);
  return (call_lowlib (PSC_FCNTL, fileid, cmd, va_arg(args, void*)));
}


