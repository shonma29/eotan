/*

B-Free Project ÇÃê∂ê¨ï®ÇÕ GNU Generic PUBLIC LICENSE Ç…è]Ç¢Ç‹Ç∑ÅB

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_utime.c,v 1.1 1997/08/31 13:25:28 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_utime.c,v 1.1 1997/08/31 13:25:28 night Exp $";


#include "../native.h"


/* utime 
 *
 */
int
utime (char *path, struct utimbuf *buf)
{
  return (call_lowlib (PSC_UTIME, strlen (path), path, buf));
}


