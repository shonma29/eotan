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
#include <stdint.h>
#include <string.h>
#include <mpufunc.h>
#include <nerve/config.h>
#include <nerve/func.h>
#include <nerve/sync.h>
#include <nerve/thread.h>
#include <nerve/icall.h>
#include <set/list.h>
#include <set/tree.h>
#include "ready.h"

static slab_t thread_slab;
static tree_t thread_tree;

static inline thread_t *getThreadParent(const node_t *);
static ER setup(thread_t *, T_CTSK *);
static void fire(thread_t *);
static void release_resources(thread_t *);


static inline thread_t *getThreadParent(const node_t *p)
{
	return ((thread_t *) ((uintptr_t) p - offsetof(thread_t, node)));
}

ER thread_initialize(void)
{
	create_tree(&thread_tree, &thread_slab, sizeof(thread_t), NULL);
	ready_initialize();
	return E_OK;
}

thread_t *get_thread_ptr(ID tskid)
{
	node_t *node = tree_get(&thread_tree, tskid);
	return (node ? getThreadParent(node) : NULL);
}

static ER setup(thread_t *th, T_CTSK *pk_ctsk)
{
	void *p;

	if (pk_ctsk->stk)
		p = pk_ctsk->stk;
	else {
		p = palloc();
		if (!p)
			return E_NOMEM;

		p += pk_ctsk->stksz;
	}

	memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

	list_initialize(&(th->queue));
	th->status = TTS_DMT;
	list_initialize(&(th->wait.waiting));
	list_initialize(&(th->locking));

	th->attr.priority = pk_ctsk->itskpri;
	th->attr.kstack_tail = (char *) kern_p2v(p);
	th->attr.ustack_top = pk_ctsk->ustack_top;
	th->attr.entry = pk_ctsk->task;
	th->attr.arg = pk_ctsk->exinf;

	context_set_kernel_sp(&(th->mpu), th->attr.kstack_tail);
	context_set_page_table(&(th->mpu),
			is_kthread(th) ?
					(VP) KTHREAD_DIR_ADDR
					//TODO null check
					: pk_ctsk->page_table);
	return E_OK;
}

ER thread_create(ID tskid, T_CTSK *pk_ctsk)
{
	if ((tskid < MIN_MANUAL_ID)
			|| (tskid > MAX_AUTO_ID))
		return E_ID;

	if (pk_ctsk->tskatr & TA_ASM)
		return E_RSATR;

	if (pk_ctsk->stksz != KTHREAD_STACK_SIZE)
		return E_PAR;

	ER result;

	enter_serialize();
	do {
		thread_t *th;
		node_t *node = slab_alloc(&thread_slab);
		if (!node) {
			result = E_NOMEM;
			break;
		}

		if (!tree_put(&thread_tree, tskid, node)) {
			slab_free(&thread_slab, node);
			result = E_OBJ;
			break;
		}

		th = getThreadParent(node);
		result = setup(th, pk_ctsk);
		if (result) {
			node = tree_remove(&thread_tree, node->key);
			if (node)
				slab_free(&thread_slab, node);
			break;
		}

		result = E_OK;

		if (pk_ctsk->tskatr & TA_ACT) {
			fire(th);
			return result;
		}
	} while (false);
	leave_serialize();

	return result;
}

static void fire(thread_t *th)
{
	th->priority = th->attr.priority;
	th->quantum = TIME_QUANTUM;
	th->port.flag = 0;

	create_context(th);

	th->status = TTS_RDY;
	ready_enqueue(th->priority, &(th->queue));
	leave_serialize_and_dispatch();
}

static void release_resources(thread_t *th)
{
	pfree((void *) ((uintptr_t) (th->attr.kstack_tail)
			- KTHREAD_STACK_SIZE));
}

ER thread_destroy(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		thread_t *th = get_thread_ptr(tskid);
		if (!th) {
			result = E_NOEXS;
			break;
		}

		if (th->status != TTS_DMT) {
			result = E_OBJ;
			break;
		}

		context_release(th);
		release_resources(th);
		node_t *node = tree_remove(&thread_tree, tskid);
		if (node)
			slab_free(&thread_slab, node);

		result = E_OK;
	} while (false);
	leave_serialize();

	return result;
}

ER thread_start(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		if (tskid == TSK_SELF)
			tskid = thread_id(running);

		thread_t *th = get_thread_ptr(tskid);
		if (!th) {
			leave_serialize();
			result = E_NOEXS;
			break;
		}

		if (th->status != TTS_DMT) {
			leave_serialize();
			result = E_QOVR;
			break;
		}

		result = E_OK;
		fire(th);
	} while (false);

	return result;
}

void thread_end(void)
{
	enter_serialize();
	list_remove(&(running->queue));
	running->status = TTS_DMT;
#ifdef USE_MUTEX
	mutex_unlock_all(running);
#endif
	leave_serialize_and_dispatch();
}

void thread_end_and_destroy(void)
{
	enter_serialize();
	icall->handle((void (*)(const int, const int)) thread_destroy,
			(const int) thread_id(running),
			0);
	thread_end();
}

ER thread_terminate(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		thread_t *th = get_thread_ptr(tskid);
		if (!th) {
			result = E_NOEXS;
			break;
		}

		switch (th->status) {
		case TTS_RUN:
			result = E_ILUSE;
			break;
		case TTS_RDY:
			list_remove(&(th->queue));
			th->status = TTS_DMT;
#ifdef USE_MUTEX
			mutex_unlock_all(th);
#endif
			result = E_OK;
			break;
		case TTS_WAI:
		case TTS_WAS:
			if (th->wait.type) {
				th->wait.type = wait_none;
				list_remove(&(th->wait.waiting));
			}
		case TTS_SUS:
			th->status = TTS_DMT;
#ifdef USE_MUTEX
			mutex_unlock_all(th);
#endif
			result = E_OK;
			break;
		default:
			result = E_OBJ;
			break;
		}
	} while (false);
	leave_serialize();

	return result;
}

ER thread_suspend(ID tskid)
{
	ER result;
//TODO test
	enter_serialize();
	do {
		thread_t *th;

		if (tskid == TSK_SELF)
			th = running;
		else {
			th = get_thread_ptr(tskid);
			if (!th) {
				result = E_NOEXS;
				break;
			}
		}

		switch (th->status) {
		case TTS_RUN:
			th->status = TTS_SUS;
			list_remove(&(th->queue));
			result = E_OK;
			if (tskid == TSK_SELF) {
				leave_serialize_and_dispatch();
				return result;
			}
			break;
		case TTS_RDY:
			th->status = TTS_SUS;
			list_remove(&(th->queue));
			result = E_OK;
			break;
		case TTS_WAI:
			th->status = TTS_WAS;
			result = E_OK;
			break;
		case TTS_SUS:
		case TTS_WAS:
			result = E_QOVR;
			break;
		default:
			result = E_OBJ;
			break;
		}
	} while (false);
	leave_serialize();

	return result;
}

ER thread_resume(ID tskid)
{
	ER result;
//TODO test
	enter_serialize();
	do {
		thread_t *th = get_thread_ptr(tskid);
		if (!th) {
			result = E_NOEXS;
			break;
		}

		switch (th->status) {
		case TTS_SUS:
			th->status = TTS_RDY;
			ready_enqueue(th->priority, &(th->queue));
			leave_serialize_and_dispatch();
			return E_OK;
		case TTS_WAS:
			th->status = TTS_WAI;
			result = E_OK;
			break;
		default:
			result = E_OBJ;
			break;
		}
	} while (false);
	leave_serialize();

	return result;
}

ID thread_get_id(void)
{
	return running->node.key;
}
