/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_getdents.c,v 1.1 2000/06/01 08:43:45 naniwa Exp $  */
static char rcsid[] = "@(#)$Id: sys_getdents.c,v 1.1 2000/06/01 08:43:45 naniwa Exp $";


#include "../native.h"


/* getdents
 *
 */
int
getdents (unsigned int fd, struct dirent *dirp, unsigned int count)
{
  return (call_lowlib (PSC_GETDENTS, fd, dirp, count));
}


