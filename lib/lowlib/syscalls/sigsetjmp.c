/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigsetjmp.c,v 1.3 1999/03/21 00:58:16 monaka Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigsetjmp.c,v 1.3 1999/03/21 00:58:16 monaka Exp $";

/*
 * $Log: sigsetjmp.c,v $
 * Revision 1.3  1999/03/21 00:58:16  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:05:00  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:50  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

int psys_sigsetjmp(void *argp)
{
#ifdef notdef
    ER error;
    struct posix_request req;
    struct posix_response res;
    struct psc_sigsetjmp *args = (struct psc_sigsetjmp *) argp;
#endif

    /*
     * まだインプリメントしていない。ENOSYS を返す。
     */
    errno = ENOSYS;

    return (NULL);
}
