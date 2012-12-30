/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header$  */

#include "../native.h"


/* brk
 *
 */
int
brk (void *endds)
{
  return (call_lowlib (PSC_BRK, endds));
}


