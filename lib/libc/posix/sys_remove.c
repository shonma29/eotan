/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_remove.c,v 1.1 1997/08/31 13:25:24 night Exp $  */

#include <string.h>
#include "../native.h"


/* remove 
 *
 */
int
remove (char *path)
{
  return (call_lowlib (PSC_REMOVE, strlen (path), path));
}


