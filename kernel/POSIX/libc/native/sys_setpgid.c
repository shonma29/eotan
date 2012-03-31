/*

B-Free Project ÇÃê∂ê¨ï®ÇÕ GNU Generic PUBLIC LICENSE Ç…è]Ç¢Ç‹Ç∑ÅB

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_setpgid.c,v 1.1 1997/08/31 13:25:24 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_setpgid.c,v 1.1 1997/08/31 13:25:24 night Exp $";


#include "../native.h"


/* setpgid 
 *
 */
int
setpgid (int gid)
{
  return (call_lowlib (PSC_SETPGID, gid));
}


