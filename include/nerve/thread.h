#ifndef _NERVE_THREAD_H_
#define _NERVE_THREAD_H_
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
#include <mpu/context.h>
#include <nerve/ipc.h>
#include <nerve/wait.h>
#include <set/list.h>
#include <set/tree.h>

typedef struct {
	node_t node;
	list_t queue;
	STAT status;
	wait_reason_t wait;
	list_t locking;
	PRI priority;
	unsigned int quantum;
	struct {
		PRI priority;
		VP kstack_tail;
		// kthread is NULL
		VP ustack_top;
		FP entry;
		VP_INT arg;
	} attr;
	ipc_t port;
	mpu_context_t mpu;
} thread_t;

static inline ID thread_id(thread_t *th)
{
	return ((ID) (th->node.key));
}

static inline thread_t *getThreadWaiting(const list_t *p)
{
	return ((thread_t *) ((uintptr_t) p
			- offsetof(thread_t, wait.waiting)));
}

#endif
