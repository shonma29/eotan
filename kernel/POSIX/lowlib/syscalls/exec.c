/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2003, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/exec.c,v 1.6 1999/11/10 10:47:25 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/exec.c,v 1.6 1999/11/10 10:47:25 naniwa Exp $";

/*
 * $Log: exec.c,v $
 * Revision 1.6  1999/11/10 10:47:25  naniwa
 * to implement execve
 *
 * Revision 1.5  1999/03/21 00:57:19  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.4  1999/02/19 10:04:32  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.3  1999/02/19 08:59:15  monaka
 * added the parameter structure on.
 *
 * Revision 1.2  1998/02/25 12:51:13  night
 * システムコール引数の定義部分を一部修正。
 *
 * Revision 1.1  1997/08/31 13:10:43  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"



/* psys_exec --- exec システムコール
 *
 *
 */
int psys_exec(void *argp)
{
    ER error;
    struct posix_request req;
    struct posix_response res;
    struct a {
	B *name;
#if 0
	 B(*argv)[];
	 B(*envp)[];
#else
	B *stackp;
	W stsize;
#endif
    } *args = (struct a *) argp;


    req.param.par_execve.pathlen = strlen(args->name);
    req.param.par_execve.name = args->name;
#if 0
    req.param.par_execve.argv = args->argv;
    req.param.par_execve.envp = args->envp;
#else
    req.param.par_execve.stackp = args->stackp;
    req.param.par_execve.stsize = args->stsize;
#endif

    error = _make_connection(PSC_EXEC, &req, &res);
    if (error != E_OK) {
	/* What should I do? */
        return (error);
    } else if (res.errno) {
	ERRNO = res.errno;
	return (-1);
    }

    return (res.status);
}
