/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_fork.c,v 1.3 1999/11/10 10:39:32 naniwa Exp $  */
static char rcsid[] = "@(#)$Id: sys_fork.c,v 1.3 1999/11/10 10:39:32 naniwa Exp $";


#include "../native.h"
#include "../../../include/arch/io.h"

/* fork 用エントリールーチン */
int fork_entry()
{
    ena_int();
    return (0);
}

/* fork 
 *
 */
int
_fork (int esp, int ebp, int ebx, int ecx, int edx, int esi, int edi)
{
  return (call_lowlib (PSC_FORK, esp, ebp, ebx, ecx, edx, esi, edi,
		       fork_entry));
}
