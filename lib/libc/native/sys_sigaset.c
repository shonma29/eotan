/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_sigaddset.c,v 1.1 1997/08/31 13:25:25 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_sigaddset.c,v 1.1 1997/08/31 13:25:25 night Exp $";


#include "../native.h"


/* sigaddset 
 *
 */
int
sigaddset (sigset_t *set, int signo)
{
  return (call_lowlib (PSC_SIGADDSET, set, signo));
}


