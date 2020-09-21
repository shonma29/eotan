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
#include <event.h>
#include <services.h>
#include <mpu/bits.h>
#include <nerve/global.h>
#include <nerve/config.h>
#include <nerve/icall.h>
#include <set/list.h>
#include "func.h"
#include "thread.h"
#include "mpu/mpufunc.h"

thread_t *running;

static list_t ready_task[MAX_PRIORITY + 1];
static unsigned int ready_bits;

static inline thread_t *getThread(const list_t *);
static void ready_rotate(const int, thread_t *);
static thread_t *ready_dequeue();


static inline thread_t *getThread(const list_t *p)
{
	return ((thread_t *) ((uintptr_t) p - offsetof(thread_t, queue)));
}

void ready_initialize(void)
{
	for (int i = MIN_PRIORITY; i <= MAX_PRIORITY; i++)
		list_initialize(&(ready_task[i]));

	ready_bits = 0;
}

void ready_enqueue(const int pri, list_t *src)
{
	list_t *dest = &(ready_task[pri]);
	ready_bits |= 1 << pri;
	list_enqueue(dest, src);
}

static void ready_rotate(const int priority, thread_t *thread)
{
	enter_serialize();

	list_t *guard = &(ready_task[priority]);
	list_t *head = list_next(guard);
	if ((head != guard)
			&& (getThread(head) == thread)
			&& (list_prev(guard) != head)) {
		list_dequeue(guard);
		list_enqueue(guard, head);
	}

	leave_serialize();
}

void tick(void)
{
	if (!is_kthread(running))
		if (!(--(running->quantum))) {
			running->quantum = TIME_QUANTUM;
			icall->handle((void (*)(const int, const int)) ready_rotate,
					(const int) (running->priority),
					(const int) running);
		}
}

static thread_t *ready_dequeue(void)
{
	while (ready_bits) {
		int pri = count_ntz(ready_bits);
		list_t *q = list_head(&(ready_task[pri]));
		if (q)
			return getThread(q);

		ready_bits &= ~(1 << pri);
	}

	return NULL;
}

void dispatch(void)
{
	enter_critical();

	do {
		if (sync_blocking)
			break;

		if (sysinfo->delay_thread_start) {
			sysinfo->delay_thread_start = false;
			ipc_notify(PORT_DELAY, EVENT_INTERRUPT);
		}

		thread_t *prev = running;
		thread_t *next = ready_dequeue();
		if (next == prev)
			break;

		if (prev->status == TTS_RUN)
			prev->status = TTS_RDY;

		next->status = TTS_RUN;
		running = next;
		context_switch(prev, next);
	} while (false);

	leave_critical();
}
