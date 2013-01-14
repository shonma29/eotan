/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_open.c,v 1.3 1999/03/15 08:36:44 monaka Exp $  */

#include <string.h>
#include "../native.h"


/* open 
 *
 */
int
open (char *path, int oflag, ...)
{
  va_list args;
  int pathLen;

  va_start(args, oflag);
  pathLen = strlen (path);
  return (call_lowlib (PSC_OPEN, pathLen, path, oflag, va_arg(args, int)));
}


