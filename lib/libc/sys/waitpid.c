/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_waitpid.c,v 1.3 1999/02/17 05:28:33 monaka Exp $  */

#include <errno.h>
#include <unistd.h>
#include "sys.h"


/* waitpid 
 *
 */
int
waitpid (pid_t pid, int *status, int option)
{
    ER error;
    pm_args_t req;
    pm_reply_t *res = (pm_reply_t*)&req;
    thread_local_t *local_data = _get_local();

    req.arg1 = pid;
    req.arg2 = (int)status;
    req.arg3 = option;

    error = _make_connection(fscall_waitpid, &req);
    if (error != E_OK) {
	local_data->error_no = error;
	return (-1);
    }
    else if (res->error_no) {
	local_data->error_no = res->error_no;
	return (-1);
    }

    if (status != NULL)
	*(status) = res->result2;
    return (res->result1);
}
