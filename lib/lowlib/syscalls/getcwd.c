/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/getcwd.c,v 1.4 1999/03/21 00:57:30 monaka Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/getcwd.c,v 1.4 1999/03/21 00:57:30 monaka Exp $";

/*
 * $Log: getcwd.c,v $
 * Revision 1.4  1999/03/21 00:57:30  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1999/02/19 10:04:34  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:19  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:44  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

extern char *strncpy2(char *, char *, int);

void *psys_getcwd(void *argp)
{
    struct psc_getcwd *args = (struct psc_getcwd *) argp;

#if 0
    ER error;
    struct posix_request req;
    struct posix_response res;

    req.param.par_getcwd.dirnamelen = args->dirnamelen;
    req.param.par_getcwd.dirname = args->dirname;

    error = _make_connection(PSC_GETCWD, &req, &res);
    if (error != E_OK) {
	/* What should I do? */
    }

    else if (res.errno) {
	ERRNO = res.errno;
	return (-1);
    }

    return (res.status);
#endif
    if (lowlib_data->dpath_len > args->dirnamelen) {
	ERRNO = EP_RANGE;
	return (NULL);
    }
    strncpy2(args->dirname, lowlib_data->dpath, args->dirnamelen);
    ERRNO = EP_OK;
    return (args->dirname);
}
