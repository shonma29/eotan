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

#include <errno.h>
#include <string.h>
#include <itron/types.h>
#include "posix.h"


/* getcwd 
 *
 */
char *
getcwd (char *buf, int size)
{
    thread_local_t *local_data = _get_local();

    if (local_data->cwd_length > size) {
	local_data->error_no = ERANGE;
	return (NULL);
    }
    strncpy(buf, (B*)(local_data->cwd), size - 1);
    buf[size - 1] = '\0';
    local_data->error_no = EOK;
    return (buf);
}


