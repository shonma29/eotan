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
#include <time.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include "api.h"
#include "../../lib/libserv/libserv.h"


static bool get_timespec(struct timespec *tspec)
{
	SYSTIM time;

	if (time_get(&time))
		return false;

	tspec->tv_sec = time.sec;
	tspec->tv_nsec = time.nsec;

	return true;
}

int mm_clock_gettime(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		struct timespec tspec;

		if (req->args.arg1 != CLOCK_REALTIME) {
			reply->data[0] = EINVAL;
			break;
		}

		if (!(req->args.arg2)) {
			reply->data[0] = EFAULT;
			break;
		}

		if (!get_timespec(&tspec)) {
			reply->data[0] = ECONNREFUSED;
			break;
		}

		if (kcall->region_put(port_of_ipc(req->node.key),
				(void*)(req->args.arg2), sizeof(tspec),
				&tspec)) {
			reply->data[0] = EFAULT;
			break;
		}

		reply->data[0] = 0;
		reply->result = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}
