/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: time.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

#define MS 1000

/* gettimeofday として動作する */

UW get_system_time(UW * usec)
{
    UW clock, i;
    SYSTIME time;

    get_tim(&time);

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
    ER errno = EP_OK;
    struct timeval tv;
    struct timezone tz;

    clock = get_system_time(&usec);

    tv.tv_sec = clock;
    tv.tv_usec = usec;
    tz.tz_minuteswest = -9 * 60;
    tz.tz_dsttime = 0;

    if (req->param.par_time.tv != NULL) {
	errno = vput_reg(req->caller,
			 req->param.par_time.tv, sizeof(struct timeval),
			 &tv);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FAIL);
	}

    }
    if (req->param.par_time.tz == NULL) {
	errno = vput_reg(req->caller,
			 req->param.par_time.tz, sizeof(struct timezone),
			 &tz);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FAIL);
	}
    }

    put_response(rdvno, EP_OK, 0, 0);
    return (SUCCESS);
}
