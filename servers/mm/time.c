/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <core.h>
#include <errno.h>
#include <mm.h>
#include <time.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include "interface.h"


static int get_timespec(struct timespec *tspec)
{
	SYSTIM time;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	if (kcall->time_get(&time))
		return FALSE;

	tspec->tv_sec = time.sec;
	tspec->tv_nsec = time.nsec;

	return TRUE;
}

int mm_clock_gettime(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		struct timespec tspec;
		kcall_t *kcall = (kcall_t*)KCALL_ADDR;

		if (args->arg1 != CLOCK_REALTIME) {
			reply->error_no = EINVAL;
			break;
		}

		if (!args->arg2) {
			reply->error_no = EFAULT;
			break;
		}

		if (!get_timespec(&tspec)) {
			reply->error_no = ECONNREFUSED;
			break;
		}

		if (kcall->region_put(get_rdv_tid(rdvno),
				(void*)(args->arg2), sizeof(tspec), &tspec)) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}
