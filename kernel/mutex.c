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
#include <nerve/config.h>
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "ready.h"
#include "sync.h"
#include "thread.h"
#include "mpu/mpufunc.h"

typedef struct {
	node_t node;
	ID locker;
	list_t waiter;
	list_t locked;
	PRI ceilpri;
} mutex_t;

static slab_t mutex_slab;
static tree_t mutex_tree;
static int mutex_hand;

static inline mutex_t *getMutexParent(const node_t *p);
static inline mutex_t *getMutexLocked(const list_t *p);
static mutex_t *get_mutex(const ID mtxid);
static void clear(mutex_t *p, const T_CMTX *pk_cmtx);
static void give(mutex_t *q);


static inline mutex_t *getMutexParent(const node_t *p) {
	return (mutex_t*)((intptr_t)p - offsetof(mutex_t, node));
}

static inline mutex_t *getMutexLocked(const list_t *p) {
	return (mutex_t*)((intptr_t)p - offsetof(mutex_t, locked));
}

static mutex_t *get_mutex(const ID mtxid)
{
	node_t *node = tree_get(&mutex_tree, mtxid);
	return (node ? getMutexParent(node) : NULL);
}

ER mutex_initialize(void)
{
	create_tree(&mutex_tree, &mutex_slab, sizeof(mutex_t), NULL);
	mutex_hand = MIN_AUTO_ID - 1;
	return E_OK;
}

static void clear(mutex_t *p, const T_CMTX *pk_cmtx) {
	p->locker = 0;
	list_initialize(&(p->waiter));
	list_initialize(&(p->locked));
	p->ceilpri = pk_cmtx->ceilpri;
}

ER mutex_create(ID mtxid, T_CMTX *pk_cmtx)
{
	if (!pk_cmtx)
		return E_PAR;
/* TODO validate pk */
	if (pk_cmtx->mtxatr != (TA_TFIFO | TA_CEILING))
		return E_RSATR;

	if ((pk_cmtx->ceilpri < MIN_PRIORITY)
			|| (pk_cmtx->ceilpri > MAX_PRIORITY))
		return E_PAR;

	ER_ID result;

	enter_serialize();
	do {
		node_t *node = slab_alloc(&mutex_slab);
		if (!node) {
			result = E_NOMEM;
			break;
		}

		if (!tree_put(&mutex_tree, mtxid, node)) {
			slab_free(&mutex_slab, node);
			result = E_OBJ;
			break;
		}

		clear(getMutexParent(node), pk_cmtx);
		result = E_OK;
	} while (false);
	leave_serialize();

	return result;
}

ER mutex_destroy(ID mtxid)
{
	ER result = mutex_lock(mtxid, TMO_FEVR);
	if (result)
		return result;

	enter_serialize();
	do {
		mutex_t *q = get_mutex(mtxid);
		if (!q) {
			result = E_NOEXS;
			break;
		}

		release_all(&(q->waiter));
		list_remove(&(q->locked));

		thread_t *th = get_thread_ptr(q->locker);
		if (list_is_empty(&(th->locking))
				&& (th->priority != th->attr.priority)) {
			th->priority = th->attr.priority;

			if (th->status & (TTS_RUN | TTS_RDY)) {
				list_remove(&(th->queue));
				ready_enqueue(th->priority, &(th->queue));
			}
		}

		node_t *node = tree_remove(&mutex_tree, mtxid);
		if (node)
			slab_free(&mutex_slab, node);
		result = E_OK;
	} while (false);
	leave_serialize();

	dispatch();
	return result;
}

ER mutex_lock(ID mtxid, TMO tmout)
{
	if ((tmout > 0)
			|| (tmout < TMO_FEVR))
		return E_PAR;

	enter_serialize();

	mutex_t *q = get_mutex(mtxid);
	if (!q) {
		leave_serialize();
		return E_NOEXS;
	}

	if (q->ceilpri < running->attr.priority) {
		leave_serialize();
		return E_ILUSE;
	}

	if (!q->locker) {
		q->locker = running->node.key;
		list_enqueue(&(running->locking), &(q->locked));

		if (q->ceilpri > running->priority) {
			list_remove(&(running->queue));
			running->priority = q->ceilpri;
			ready_enqueue(running->priority, &(running->queue));
		}

		leave_serialize();
		return E_OK;
	}

	if (q->locker == running->node.key) {
		leave_serialize();
		return E_ILUSE;
	}

	if (tmout == TMO_POL) {
		leave_serialize();
		return E_TMOUT;
	}

	list_enqueue(&(q->waiter), &(running->wait.waiting));
	running->wait.type = wait_mtx;
	wait(running);
	return running->wait.result;
}

ER mutex_unlock(ID mtxid)
{
	enter_serialize();

	mutex_t *q = get_mutex(mtxid);
	if (!q) {
		leave_serialize();
		return E_NOEXS;
	}

	if (q->locker != running->node.key) {
		leave_serialize();
		return E_ILUSE;
	}

	list_remove(&(q->locked));
	if (list_is_empty(&(running->locking)))
		if (running->priority != running->attr.priority) {
			list_remove(&(running->queue));
			running->priority = running->attr.priority;
			ready_enqueue(running->priority, &(running->queue));
		}

	give(q);
	leave_serialize();

	dispatch();
	return E_OK;
}

void mutex_unlock_all(thread_t *th)
{
	for (;;) {
		list_t *p = list_head(&(th->locking));
		if (!p)
			break;

		list_remove(p);
		give(getMutexLocked(p));
	}
}

static void give(mutex_t *q)
{
	list_t *waiter = list_head(&(q->waiter));
	if (waiter) {
		thread_t *tp = getThreadWaiting(waiter);

		q->locker = tp->node.key;
		list_enqueue(&(tp->locking), &(q->locked));
		list_remove(waiter);

		if (q->ceilpri > tp->priority)
			tp->priority = q->ceilpri;

		release(tp);

	} else
		q->locker = 0;
}
