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
#include <itron/types.h>
#include <lowlib.h>
#include "posix.h"
#include "../libserv/libserv.h"


/* chdir 
 *
 */
int
chdir (char *path)
{
    struct lowlib_data	*lowlib_data = (struct lowlib_data *)LOWLIB_DATA;
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;
    char buf[MAX_DPATH + 1], *src, *top, *dst;
    int len = strlen(path), l2, flag = 0;

    req.param.par_chdir.path = path;
    req.param.par_chdir.pathlen = len;

    error = _make_connection(PSC_CHDIR, &req);
    if (error != E_OK) {
	/* What should I do? */
    }

    else if (res->errno) {
	errno = res->errno;
	return (-1);
    }

    /* lowlib_data->dpath の更新 */
    if (path[0] == '/') {
	strncpy2(lowlib_data->dpath, path, MAX_DPATH + 1);
	lowlib_data->dpath_len = len;
    } else {
	strncpy2(buf, path, MAX_DPATH + 1);
	top = src = buf;
	len = lowlib_data->dpath_len;
	dst = lowlib_data->dpath + len;
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
		if (len + l2 >= MAX_DPATH) {
		    l2 = MAX_DPATH - len - 1;
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
	lowlib_data->dpath_len = len;
    }

    return (res->status);
}


