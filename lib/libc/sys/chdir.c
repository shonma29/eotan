/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_chdir.c,v 1.1 1997/08/31 13:25:18 night Exp $  */

#include <errno.h>
#include <string.h>
#include <core/types.h>
#include "posix.h"
#include "../libserv/libserv.h"


/* chdir 
 *
 */
int
chdir (char *path)
{
    thread_local_t *local_data = _get_local();
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;
    char buf[MAX_CWD + 1], *src, *top, *dst;
    int len = strlen(path), l2, flag = 0;

    req.param.par_chdir.path = path;
    req.param.par_chdir.pathlen = len;

    error = _make_connection(PSC_CHDIR, &req);
    if (error != E_OK) {
	local_data->error_no = error;
	return (-1);
    }

    else if (res->error_no) {
	local_data->error_no = res->error_no;
	return (-1);
    }

    /* lowlib_data->dpath の更新 */
    if (path[0] == '/') {
	strncpy((B*)(local_data->cwd), path, MAX_CWD);
	local_data->cwd[MAX_CWD] = '\0';
	local_data->cwd_length = len;
    } else {
	strncpy(buf, path, MAX_CWD);
	buf[MAX_CWD] = '\0';
	top = src = buf;
	len = local_data->cwd_length;
	dst = (B*)(local_data->cwd + len);
	while (*top) {
	    while (*src != '/' && *src)
		++src;
	    if (*src)
		*src++ = 0;
	    if (!strcmp(top, "..")) {
		if (len > 1) {
		    while (*dst != '/') {
			--dst;
			--len;
		    }
		    if (len == 0) {
			++len;
			*(dst + 1) = 0;
		    } else
			*dst = 0;
		}
	    } else if (strcmp(top, ".")) {
		l2 = strlen(top);
		if (len + l2 >= MAX_CWD) {
		    l2 = MAX_CWD - len - 1;
		    flag = 1;
		}
		if (len != 1) {
		    *dst++ = '/';
		    ++len;
		}
		len += l2;
		while (l2-- > 0)
		    *dst++ = *top++;
		*dst = 0;
		if (flag)
		    break;
	    }
	    top = src;
	}
	local_data->cwd_length = len;
    }

    return (res->status);
}


