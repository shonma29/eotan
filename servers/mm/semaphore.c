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
#include <sys/errno.h>
#include <mm/config.h>
#include <mpufunc.h>
#include <nerve/ipc_utils.h>
#include "../../lib/libserv/libserv.h"
#include "process.h"
#include "semaphore.h"

enum {
	SEMAPHORE_SLAB = 0x01
};

static sys_reply_t async_reply = { 1, { 0, 0 } };
static slab_t semaphore_slab;
static tree_t semaphore_tree;
static int initialized_resources = 0;

static inline mm_thread_t *getThreadFromWaiting(const list_t *p)
{
	return ((mm_thread_t *) ((uintptr_t) p
			- offsetof(mm_thread_t, waiting)));
}

static bool _includes(const mm_segment_t *, const void *);
static int _find(mm_semaphore_t **, const void *, mm_process_t *);
static void _release(mm_semaphore_t *);


int semaphore_initialize(void)
{
	if (create_slab(&semaphore_slab, sizeof(mm_semaphore_t), SEMAPHORE_MAX))
		return SEMAPHORE_SLAB;
	else
		initialized_resources |= SEMAPHORE_SLAB;

	tree_create(&semaphore_tree, NULL, NULL);
	return 0;
}

void semaphore_destroy(mm_semaphore_t *sem)
{
	tree_remove(&semaphore_tree, sem->node.key);
	slab_free(&semaphore_slab, sem);
}

static int _find(mm_semaphore_t **s, const void *address,
		mm_process_t *process)
{
	mm_segment_t *segment = process->segments.bss;
	if (!segment
			|| !_includes(segment, address)) {
		segment = process->segments.heap;
		if (!segment
				|| !_includes(segment, address))
			return EINVAL;
	}

	uintptr_t paddr = (uintptr_t) get_writable_page(
			kern_p2v(process->directory),
			address);
	if (!paddr)
		return EFAULT;

	paddr |= getOffset(address);

	node_t *node = tree_get(&semaphore_tree, paddr);
	if (node) {
		*s = getSemaphoreFromNode(node);
		return 0;
	}

	mm_semaphore_t *p = (mm_semaphore_t *) slab_alloc(&semaphore_slab);
	if (!p)
		return ((semaphore_slab.block_num >= semaphore_slab.max_block) ?
				ENOSPC : ENOMEM);

	if (!tree_put(&semaphore_tree, paddr, &(p->node))) {
		slab_free(&semaphore_slab, p);
		return EEXIST;
	}

	list_initialize(&(p->threads));
	list_append(&(segment->semaphores), &(p->segments));
	p->count = 0;
	*s = p;
	return 0;
}

static bool _includes(const mm_segment_t *segment, const void *address)
{
	uintptr_t start = (uintptr_t) (segment->addr);
	uintptr_t end = ((uintptr_t) (segment->addr) + segment->len);
	return (((uintptr_t) address >= start)
			&& ((uintptr_t) address < end));
}

static void _release(mm_semaphore_t *s)
{
	for (; s->count > 0; s->count--) {
		list_t *head = list_pick(&(s->threads));
		if (!head)
			break;

		mm_thread_t *th = getThreadFromWaiting(head);
		int result = kcall->ipc_send(th->wait.tag, &async_reply,
				sizeof(async_reply));
		if (result)
			log_warning("mm: semaphore reply error %x %d\n",
					get_process(th)->node.key, result);

		th->wait.obj = 0;
		th->wait.reason = REASON_NONE;
		th->wait.tag = 0;
	}
}

int mm_semacquire(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_semaphore_t *s;
		int result = _find(&s, (void *) (req->args.arg1), process);
		if (result) {
			reply->data[0] = result;
			break;
		}

		if (s->count > 0) {
			s->count--;
			reply->result = 1;
		} else if (req->args.arg2) {
			th->wait.obj = (uintptr_t) s;
			th->wait.reason = REASON_SEMAPHORE;
			th->wait.tag = req->node.key;
			list_append(&(s->threads), &(th->waiting));
			return reply_wait;
		} else
			reply->result = 0;

		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_semrelease(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_semaphore_t *s;
		int result = _find(&s, (void *) (req->args.arg1), process);
		if (result) {
			reply->data[0] = result;
			break;
		}

		if (s->count > 0) {
			if (req->args.arg2 > LONG_MAX - s->count) {
				reply->data[0] = EINVAL;
				break;
			}
		} else {
			if (req->args.arg2 < LONG_MIN - s->count) {
				reply->data[0] = EINVAL;
				break;
			}
		}

		s->count += req->args.arg2;
		reply->data[0] = 0;
		reply->result = s->count;
		_release(s);
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}
