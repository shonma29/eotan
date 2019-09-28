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
#include <services.h>
#include <mpu/memory.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "process.h"

static void expand_stack(const int tid, const int addr);
static void kill(const int tid, const int dummy);


ER default_handler(void)
{
	if (icall->handle(kill, icall->thread_get_id(), 0))
		//TODO test
		panic("mm: full kqueue");

	return E_OK;
}

ER stack_fault_handler(void)
{
	if (icall->handle(expand_stack, icall->thread_get_id(),
			(int) fault_get_addr()))
		//TODO test
		panic("mm: full kqueue");

	return E_OK;
}

static void expand_stack(const int tid, const int addr)
{
	mm_thread_t *th = thread_find(tid);
	if (th) {
		mm_process_t *p = get_process(th);
		if (!p) {
			//TODO test
			log_warning("mm: expand no parent=%d\n", tid);
			return;
		}

		mm_segment_t *s = &(p->segments.stack);
		void *start = (void *) pageRoundDown((UW) addr);
		if (s->attr
				&& ((size_t) start >= (size_t) (s->addr)
				&& ((size_t) addr <= ((size_t) (s->addr) + s->max)))) {
			size_t size = pages(PAGE_SIZE);

			if (map_user_pages(p->directory, start, size))
				//TODO test
				unmap_user_pages(p->directory, start, size);
			else {
				//TODO test
				tlb_flush_all();
				return;
			}
		}

		//TODO test
		kill(tid, 0);
	} else
		//TODO test
		log_warning("mm: expand unknown thread=%d\n", tid);
}

static void kill(const int tid, const int dummy)
{
	kcall->thread_suspend(tid);

	mm_thread_t *th = thread_find(tid);
	if (!th) {
		log_warning("mm: kill unknown thread=%d\n", tid);
		return;
	}

	mm_process_t *p = get_process(th);
	if (p)
		process_destroy(p, 9);
}
