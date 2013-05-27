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

#include <kcall.h>
#include "fs.h"

#define MS 1000

extern void sfs_purge_cache();
extern void print_freelist();

W psc_misc_f(RDVNO rdvno, struct posix_request *req)
{
    ER err = 0;

#ifdef notdef
    printf("POSIX: misc systemcall.(cmd = %d)\n", req->param.par_misc.cmd);
#endif

    switch (req->param.par_misc.cmd) {
    case M_PROC_DUMP:
	err = proc_dump(req);
	break;
#if 0
    case M_VM_DUMP:
	err = proc_vm_dump(req);
	break;
#endif
    case M_PRINT_FLIST:
	/* malloc の空きリストの表示 */
	print_freelist();
	break;

    case M_PURGE_CACHE:
	sfs_purge_cache();
	break;

    case M_DO_PS:
	err = do_ps();
	break;

    case M_DO_DF:
	err = do_df();
	break;
    }

    put_response(rdvno, err, err ? -1 : 0, 0);
    if (err)
	return (FALSE);
    else
	return (TRUE);
}

/* gettimeofday として動作する */

UW get_system_time(UW * usec)
{
    UW clock, i;
    SYSTIM time;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    kcall->time_get(&time);

#ifdef notdef
    dbg_printf("systime.utime = %d, systime.ltime = %d\n",
	       time.utime, time.ltime);
#endif
    clock = time.ltime / MS;
    i = (time.utime % MS) << 16;
    clock += (i / MS) << 16;
    clock += ((i % MS) << 16) / MS;
    if (usec != NULL) {
	*usec = (time.ltime % MS) / 10;
    }
    return clock;
}

W psc_time_f(RDVNO rdvno, struct posix_request * req)
{
    UW clock, usec;
    ER error_no = EOK;
    struct timeval tv;
    struct timezone tz;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    clock = get_system_time(&usec);

    tv.tv_sec = clock;
    tv.tv_usec = usec;
    tz.tz_minuteswest = -9 * 60;
    tz.tz_dsttime = 0;

    if (req->param.par_time.tv != NULL) {
	error_no = kcall->region_put(req->caller,
			 req->param.par_time.tv, sizeof(struct timeval),
			 &tv);
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return (FALSE);
	}

    }
    if (req->param.par_time.tz == NULL) {
	error_no = kcall->region_put(req->caller,
			 req->param.par_time.tz, sizeof(struct timezone),
			 &tz);
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return (FALSE);
	}
    }

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
