/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_exit.c,v 1.3 2000/02/16 08:14:50 naniwa Exp $  */
static char rcsid[] = "@(#)$Id: sys_exit.c,v 1.3 2000/02/16 08:14:50 naniwa Exp $";


#include "../native.h"


/* _exit 
 *
 */
int
_exit (W status)
{
  return (call_lowlib (PSC_EXIT, status));
}


