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

#include <errno.h>
#include "posix.h"

/* fork 用エントリールーチン */
extern int _fork_entry();

/* fork 
 *
 */
int
_fork (int esp, int ebp, int ecx, int edx, int esi, int edi)
{
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;

    /* POSIX manager の呼び出し 

     * 引数を設定して、POSIX manager にメッセージを送る。
     */
    req.param.par_fork.sp = (VP)esp;
    req.param.par_fork.entry = (FP)_fork_entry;

    error = _make_connection(PSC_FORK, &req);
    if (error != E_OK) {
	/* What should I do? */
	errno = error;
	return (-1);
    } else if (res->errno) {
	errno = res->errno;
	return (-1);
    }

    return (res->status);
}
