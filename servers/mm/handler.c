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
#include <sys/unistd.h>
#include <sys/signal.h>
#include <mpufunc.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include "process.h"

static void expand_stack(const int, const int);
static void kill(const int, const int);


void default_handler(VP_INT exinf)
{
	//TODO map to signal no
	if (icall->handle(kill, icall->thread_get_id(), (int) exinf))
		//TODO test
		panic("mm: full kqueue");
}

void page_fault_handler(VP_INT exinf)
{
	if (icall->handle(expand_stack, icall->thread_get_id(),
			(int) fault_get_addr()))
		//TODO test
		panic("mm: full kqueue");
}

static void expand_stack(const int tid, const int addr)
{
	//TODO lock thread/process table
	mm_thread_t *th = thread_find(tid);
	if (th) {
		mm_process_t *p = get_process(th);
		if (!p) {
			//TODO test
			kcall->printk("mm: expand no parent=%d\n", tid);
			return;
		}

		mm_segment_t *s = &(th->stack);
		void *start = (void *) pageRoundDown((uintptr_t) addr);
		if (s->attr
				&& ((size_t) start >= (size_t) (s->addr)
				&& ((size_t) addr < ((size_t) (s->addr)
						+ s->max)))) {
			uintptr_t end = (uintptr_t) s->addr + s->max - s->len;
			size_t size = pages(end - (uintptr_t) start);
			if (map_user_pages(p->directory, start, size,
					R_OK | W_OK))
				//TODO test
				//TODO skip holes on recovery
				unmap_user_pages(p->directory, start, size);
			else {
				s->len += end - (uintptr_t) start;
				//TODO optimize
				tlb_flush_all();
				return;
			}
		}

		kcall->printk(MYNAME ": page fault %x\n", addr);
		kill(tid, SIGSEGV);
	} else
		//TODO test
		kcall->printk("mm: expand unknown thread=%d addr=%p\n",
				tid, addr);
}

static void kill(const int tid, const int no)
{
	kcall->thread_suspend(tid);

	//TODO use not thread id but process id
	sys_args_t args = { syscall_kill, tid, no };
	kcall->ipc_send(PORT_MM, &args, sizeof(args));
}
