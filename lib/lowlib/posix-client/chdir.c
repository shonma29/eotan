/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/chdir.c,v 1.4 1999/03/21 00:57:10 monaka Exp $ */

/*
 * $Log: chdir.c,v $
 * Revision 1.4  1999/03/21 00:57:10  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1999/02/19 10:04:28  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:07  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:42  night
 * 最初の登録
 *
 *
 *
 */

#include <string.h>
#include "../lowlib.h"

extern char *strncpy2(char *, char *, int);

int psys_chdir(void *argp)
{
    ER error;
    struct posix_request req;
    struct posix_response res;
    struct psc_chdir *args = (struct psc_chdir *) argp;
    char buf[MAX_DPATH + 1], *src, *top, *dst;
    int len, l2, flag = 0;

    req.param.par_chdir.path = args->path;
    req.param.par_chdir.pathlen = args->pathlen;

    error = _make_connection(PSC_CHDIR, &req, &res);
    if (error != E_OK) {
	/* What should I do? */
    }

    else if (res.errno) {
	ERRNO = res.errno;
	return (-1);
    }

    /* lowlib_data->dpath の更新 */
    if (args->path[0] == '/') {
	strncpy2(lowlib_data->dpath, args->path, MAX_DPATH + 1);
	lowlib_data->dpath_len = args->pathlen;
    } else {
	strncpy2(buf, args->path, MAX_DPATH + 1);
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

    return (res.status);
}
