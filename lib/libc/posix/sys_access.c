/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_access.c,v 1.1 1997/08/31 13:25:17 night Exp $  */

#include <string.h>
#include "../native.h"


/* access 
 *
 */
int
access (char *path, int mode)
{
  return (call_lowlib (PSC_ACCESS, strlen (path), path, mode));
}


