/*

B-Free Project ÇÃê∂ê¨ï®ÇÕ GNU Generic PUBLIC LICENSE Ç…è]Ç¢Ç‹Ç∑ÅB

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_noaction.c,v 1.1 1997/08/31 13:25:23 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_noaction.c,v 1.1 1997/08/31 13:25:23 night Exp $";


#include "../native.h"


/* noaction
 *
 */
int
noaction ()
{
  return (call_lowlib (PSC_NOACTION));
}


