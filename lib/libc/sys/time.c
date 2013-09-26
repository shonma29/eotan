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

#include <time.h>
#include <fs.h>
#include "posix.h"


/* gettimeofday -- POSIX システムコールの動作はこちらに変更
 *
 */
int gettimeofday(struct timeval *tv, struct timezone * tz)
{
    struct posix_request req;

    req.param.par_time.tv = tv;
    req.param.par_time.tz = tz;

    return _call_fs(PSC_TIME, &req);
}

/* time 
 *
 */
time_t time(time_t * tm)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    if (tm != NULL) {
	*tm = tv.tv_sec;
    }
    return tv.tv_sec;
}
