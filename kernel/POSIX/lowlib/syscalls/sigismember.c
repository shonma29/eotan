/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigismember.c,v 1.3 1999/03/21 00:58:12 monaka Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigismember.c,v 1.3 1999/03/21 00:58:12 monaka Exp $";

/*
 * $Log: sigismember.c,v $
 * Revision 1.3  1999/03/21 00:58:12  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:56  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:49  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

int psys_sigismember(void *argp)
{
#ifdef notdef
    ER error;
    struct posix_request req;
    struct posix_response res;
    struct psc_sigismember *args = (struct psc_sigismember *) argp;
#endif

    /*
     * まだインプリメントしていない。ENOSYS を返す。
     */
    errno = ENOSYS;

    return (NULL);
}
