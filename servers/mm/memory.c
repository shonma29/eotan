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
#include <core/options.h>
#include "process.h"
#include "../../kernel/mpu/mpufunc.h"


int mm_vmap(mm_request *req)
{
	mm_reply_t *reply = (mm_reply_t *) &(req->args);
	do {
		mm_process_t *p = process_find((ID)(req->args.arg1));
		if (!p) {
			reply->data[0] = ESRCH;
			break;
		}

		if (map_user_pages(p->directory,
				(VP)(req->args.arg2),
				pages((UW)(req->args.arg3)))) {
			reply->data[0] = ENOMEM;
			break;
		}

		unsigned int currentEnd = (unsigned int)(p->segments.heap.addr)
				+ p->segments.heap.len;
		unsigned int newEnd = (unsigned int)(req->args.arg2)
				+ (unsigned int)(req->args.arg3);
		if (currentEnd == (unsigned int)(req->args.arg2))
			p->segments.heap.len = newEnd
					- (unsigned int)(p->segments.heap.addr);

		if (req->args.arg2 == LOCAL_ADDR)
			p->local = getPageAddress(kern_p2v(p->directory),
					(void*)(req->args.arg2));

		reply->data[0] = 0;
		reply->result = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_vunmap(mm_request *req)
{
	mm_reply_t *reply = (mm_reply_t *) &(req->args);
	do {
		unsigned int currentEnd;
		unsigned int newEnd;
		mm_process_t *p = process_find((ID)(req->args.arg1));

		if (!p) {
			reply->data[0] = ESRCH;
			break;
		}

		if (unmap_user_pages(p->directory,
				(VP)(req->args.arg2),
				pages((UW)(req->args.arg3)))) {
			reply->data[0] = EACCES;
			break;
		}

		currentEnd = (unsigned int)(p->segments.heap.addr)
				+ p->segments.heap.len;
		newEnd = (unsigned int)(req->args.arg2)
				+ (unsigned int)(req->args.arg3);
		if (currentEnd == newEnd)
			p->segments.heap.len = (unsigned int)(req->args.arg2)
					- (unsigned int)(p->segments.heap.addr);

		reply->data[0] = 0;
		reply->result = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_sbrk(mm_request *req)
{
	mm_reply_t *reply = (mm_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *p = get_process(th);
		mm_segment_t *s = &(p->segments.heap);
		uintptr_t end = (uintptr_t)(s->addr) + s->len;
		intptr_t diff = (intptr_t)(req->args.arg1);
		if (diff > 0) {
			diff = pageRoundUp(diff);
			if (s->max - s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (map_user_pages(p->directory,
					(VP)end, pages(diff))) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len += diff;
			end += diff;

		} else if (diff < 0) {
			diff = pageRoundUp(-diff);
			if (s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (unmap_user_pages(p->directory,
					(VP)(end - diff), pages(diff))) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len -= diff;
			end -= diff;
		}

		reply->data[0] = 0;
		reply->result = (int)end;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}
