/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: misc.c,v $
 * Revision 1.2  2000/01/30 09:10:15  naniwa
 * add M_PRINT_FLIST, and to call vsys_msc
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <nerve/kcall.h>


/* gettimeofday として動作する */
UW get_system_time(void)
{
    SYSTIM time;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    kcall->time_get(&time);

    return (UW)((unsigned long long)time.sec & 0xffffffff);
}
