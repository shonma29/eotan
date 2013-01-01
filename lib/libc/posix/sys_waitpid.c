/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_waitpid.c,v 1.3 1999/02/17 05:28:33 monaka Exp $  */

#include <unistd.h>
#include "../native.h"


/* waitpid 
 *
 */
int
waitpid (pid_t pid, int *status, int option)
{
  return (call_lowlib (PSC_WAITPID, pid, status, option));
}


