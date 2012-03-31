/*

B-Free Project ÇÃê∂ê¨ï®ÇÕ GNU Generic PUBLIC LICENSE Ç…è]Ç¢Ç‹Ç∑ÅB

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_open.c,v 1.3 1999/03/15 08:36:44 monaka Exp $  */
static char rcsid[] = "@(#)$Id: sys_open.c,v 1.3 1999/03/15 08:36:44 monaka Exp $";


#include "../native.h"


/* open 
 *
 */
int
open (char *path, int oflag, int mode)
{
  int pathLen;

  pathLen = strlen (path);
  return (call_lowlib (PSC_OPEN, pathLen, path, oflag, mode));
}


