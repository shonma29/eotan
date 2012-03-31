/*

B-Free Project ÇÃê∂ê¨ï®ÇÕ GNU Generic PUBLIC LICENSE Ç…è]Ç¢Ç‹Ç∑ÅB

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_sigsuspend.c,v 1.4 1999/02/17 05:28:32 monaka Exp $  */
static char rcsid[] = "@(#)$Id: sys_sigsuspend.c,v 1.4 1999/02/17 05:28:32 monaka Exp $";


#include "../native.h"


/* sigsuspend 
 *
 */
int
sigsuspend (void)
{
  return (call_lowlib (PSC_SIGSUSPEND));
}


