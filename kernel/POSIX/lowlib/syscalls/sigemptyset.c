/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigemptyset.c,v 1.3 1999/03/21 00:58:10 monaka Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sigemptyset.c,v 1.3 1999/03/21 00:58:10 monaka Exp $";

/*
 * $Log: sigemptyset.c,v $
 * Revision 1.3  1999/03/21 00:58:10  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:54  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:49  night
 * �ǽ����Ͽ
 *
 *
 *
 */


#include "../lowlib.h"

int psys_sigemptyset(void *argp)
{
#ifdef notdef
    ER error;
    struct posix_request req;
    struct posix_response res;
    struct psc_sigemptyset *args = (struct psc_sigemptyset *) argp;
#endif

    /*
     * �ޤ�����ץ���Ȥ��Ƥ��ʤ���ENOSYS ���֤���
     */
    errno = ENOSYS;

    return (NULL);
}
