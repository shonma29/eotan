/*

B-Free Project �̐������� GNU Generic PUBLIC LICENSE �ɏ]���܂��B

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_chdir.c,v 1.1 1997/08/31 13:25:18 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_chdir.c,v 1.1 1997/08/31 13:25:18 night Exp $";


#include "../native.h"


/* chdir 
 *
 */
int
chdir (char *path)
{
  return (call_lowlib (PSC_CHDIR, strlen (path), path));
}


