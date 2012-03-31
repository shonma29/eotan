/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_getcwd.c,v 1.1 1997/08/31 13:25:20 night Exp $  */
static char rcsid[] = "@(#)$Id: sys_getcwd.c,v 1.1 1997/08/31 13:25:20 night Exp $";


#include "../native.h"


/* getcwd 
 *
 */
char *
getcwd (char *buf, int size)
{
  return ((char *) call_lowlib (PSC_GETCWD, buf, size));
}


