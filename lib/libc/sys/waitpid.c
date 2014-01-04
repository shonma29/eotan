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
#include "posix.h"


/* waitpid 
 *
 */
int
waitpid (pid_t pid, int *status, int option)
{
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;
    thread_local_t *local_data = _get_local();

    req.param.par_waitpid.pid = pid;
    req.param.par_waitpid.statloc = (W*)status;
    req.param.par_waitpid.opts = option;

    error = _make_connection(PSC_WAITPID, &req);
    if (error != E_OK) {
	local_data->error_no = error;
	return (-1);
    }
    else if (res->error_no) {
	local_data->error_no = res->error_no;
	return (-1);
    }

    if (status != NULL)
	*(status) = res->ret1;
    return (res->status);
}
