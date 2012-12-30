/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_dup2.c,v 1.1 1997/08/31 13:25:19 night Exp $  */

#include "../native.h"


/* dup2 
 *
 */
int
dup2 (int fd, int fd2)
{
  return (call_lowlib (PSC_DUP2, fd, fd2));
}


