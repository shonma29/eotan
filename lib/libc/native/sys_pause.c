/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_pause.c,v 1.1 1997/08/31 13:25:23 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_pause.c,v 1.1 1997/08/31 13:25:23 night Exp $";


#include "../native.h"


/* pause 
 *
 */
int
pause (void)
{
  return (call_lowlib (PSC_PAUSE));
}

