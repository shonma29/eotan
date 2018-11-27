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
#include "sys.h"

/* fork 用エントリールーチン */
extern int _fork_entry();

/* fork 
 *
 */
int
_fork (int esp, int ebx, int ebp, int esi, int edi)
{
    ER error;
    pm_args_t req;
    pm_reply_t *res = (pm_reply_t*)&req;
    thread_local_t *local_data = _get_local();

    /* POSIX manager の呼び出し 

     * 引数を設定して、POSIX manager にメッセージを送る。
     */
    req.arg1 = esp;
    req.arg2 = (int)_fork_entry;

    error = _make_connection(fscall_fork, &req);
    if (error != E_OK) {
	local_data->error_no = error;
	return (-1);
    } else if (res->error_no) {
	local_data->error_no = res->error_no;
	return (-1);
    }

    return (res->result1);
}
