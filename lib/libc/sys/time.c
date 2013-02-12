/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_time.c,v 1.1 1997/08/31 13:25:27 night Exp $  */

#include <errno.h>
#include <time.h>
#include "posix.h"


/* gettimeofday -- POSIX システムコールの動作はこちらに変更
 *
 */
int gettimeofday(struct timeval *tv, struct timezone * tz)
{
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;

    req.param.par_time.tv = tv;
    req.param.par_time.tz = tz;

    error = _make_connection(PSC_TIME, &req);
    if (error != E_OK) {
	/* What should I do? */
    }

    else if (res->errno) {
	errno = res->errno;
	return (-1);
    }

    return (res->status);
}

/* time 
 *
 */
int time(time_t * tm)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    if (tm != NULL) {
	*tm = tv.tv_sec;
    }
    return tv.tv_sec;
}
