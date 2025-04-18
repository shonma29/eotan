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
#include <stdbool.h>
#include <nerve/func.h>
#include <nerve/sync.h>
#include <nerve/thread.h>
#include "ready.h"


void wait(thread_t *task)
{
	task->wait.result = E_OK;
	task->status = TTS_WAI;
	list_remove(&(task->queue));

	leave_serialize_and_dispatch();
}

void release(thread_t *task)
{
	task->wait.type = wait_none;
	task->status &= ~TTS_WAI;

	if (!task->status) {
		task->status = TTS_RDY;
		ready_enqueue(task->priority, &(task->queue));
	}
}

void release_all(list_t *waiting) {
	list_t *q;

	while ((q = list_dequeue(waiting)) != NULL) {
		thread_t *p = getThreadWaiting(q);

		p->wait.result = E_DLT;
		release(p);
/* TODO test */
	}
}

void leave_serialize(void)
{
	sysinfo->sync.state.serializing = 0;
	if (!(sysinfo->sync.dispatch_skippable))
		dispatch();
}

void leave_serialize_and_dispatch(void)
{
	sysinfo->sync.state.no_request = 0;
	leave_serialize();
}
