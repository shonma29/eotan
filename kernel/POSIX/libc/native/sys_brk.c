/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header$  */
static char rcsid[] = "@(#)$Id$";


#include "../native.h"


/* brk
 *
 */
int
brk (void *endds)
{
  return (call_lowlib (PSC_BRK, endds));
}


