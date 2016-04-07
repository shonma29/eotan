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
#include <nerve/global.h>
#include <nerve/config.h>
#include <set/list.h>
#include <set/heap.h>
#include "func.h"
#include "thread.h"
#include "mpu/mpufunc.h"

thread_t *running;

static list_t ready_task[MAX_PRIORITY + 1];
static int buf[MAX_PRIORITY + 1];
static heap_t heap;

static inline thread_t *getThread(const list_t *p);
static thread_t *ready_dequeue();


static inline thread_t *getThread(const list_t *p) {
	return (thread_t*)((intptr_t)p - offsetof(thread_t, queue));
}

void ready_initialize()
{
	int i;

	for (i = MIN_PRIORITY; i <= MAX_PRIORITY; i++) {
		list_initialize(&(ready_task[i]));
	}

	heap_initialize(&heap, MAX_PRIORITY + 1, buf);
}

void ready_enqueue(const int pri, list_t *src)
{
	list_t *dest = &(ready_task[pri]);

	if (list_is_empty(dest))	heap_enqueue(&heap, pri);
	list_enqueue(dest, src);
}

void ready_rotate(const int pri)
{
	list_t *head = list_dequeue(&(ready_task[pri]));

	if (head) {
		list_enqueue(&(ready_task[pri]), head);
	}
}

static thread_t *ready_dequeue()
{
	for (;; heap_dequeue(&heap)) {
		list_t *q;
		int pri = heap_head(&heap);

		if (pri == HEAP_EMPTY)	return NULL;

		q = list_head(&(ready_task[pri]));

		if (q)	return getThread(q);
	}
}

void dispatch(void)
{
	enter_critical();

	if (sync_blocking) {
		leave_critical();
		return;
	}

	if (sysinfo->delay_thread_start) {
		sysinfo->delay_thread_start = FALSE;
		thread_wakeup(sysinfo->delay_thread_id);
	}

	do {
		thread_t *p = running;
		thread_t *q = ready_dequeue();

		if (q == p) {
			leave_critical();
			break;
		}

		if (p->status == TTS_RUN)
			p->status = TTS_RDY;

		q->status = TTS_RUN;
		running = q;

		context_switch(p, q);
	} while (false);
}
