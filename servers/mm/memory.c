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
#include <errno.h>
#include <sys/unistd.h>
#include <mpufunc.h>
#include <nerve/ipc_utils.h>
#include "api.h"
#include "process.h"


int mm_segattach(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		reply->data[0] = 0;
		reply->result = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_segdetach(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		reply->data[0] = 0;
		reply->result = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_sbrk(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *p = get_process(th);
		mm_segment_t *s = p->segments.heap;
		if (!s) {
			int result = process_allocate_heap(p);
			if (result) {
				reply->data[0] = result;
				break;
			}

			s = p->segments.heap;
		}

		uintptr_t end = (uintptr_t) (s->addr) + s->len;
		intptr_t diff = (intptr_t) (req->args.arg1);
		if (diff > 0) {
			diff = pageRoundUp(diff);
			if (s->max - s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (map_user_pages(p->directory,
					(VP) end, pages(diff), R_OK | W_OK)) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len += diff;
		} else if (diff < 0) {
			diff = pageRoundUp(-diff);
			if (s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (unmap_user_pages(p->directory,
					(VP) (end - diff), pages(diff))) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len -= diff;
		}

		reply->data[0] = 0;
		reply->result = (int) end;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}
