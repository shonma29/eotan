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
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "sync.h"
#include "setting.h"
#include "thread.h"
#include "mpu/mpu.h"
#include "mpu/mpufunc.h"

#define MAX_DTQCNT (1024 - 1)

typedef struct {
	node_t node;
	list_t sender;
	list_t receiver;
	UINT dtqcnt;
	UINT *dtq;
	UINT read;
	UINT write;
	UINT left;
} queue_t;

static slab_t queue_slab;
static tree_t queue_tree;
static int queue_hand;

static inline queue_t *getQueueParent(const node_t *p);
static inline queue_t *getSenderParent(const list_t *p);
static inline queue_t *getReceiverParent(const list_t *p);
static inline thread_t *getTaskParent(const list_t *p);
static inline int is_full(queue_t *q);
static inline int is_empty(queue_t *q);
static void enqueue(queue_t *q, VP_INT data);
static VP_INT dequeue(queue_t *q);
static queue_t *get_queue(const ID dtqid);
static list_t *release_sender(VP_INT *d, queue_t *q);
static void clear(queue_t *p, const T_CDTQ *pk_cdtq);
static void release_all(list_t *waiting);


static inline queue_t *getQueueParent(const node_t *p) {
	return (queue_t*)((ptr_t)p - offsetof(queue_t, node));
}

static inline queue_t *getSenderParent(const list_t *p) {
	return (queue_t*)((ptr_t)p - offsetof(queue_t, sender));
}

static inline queue_t *getReceiverParent(const list_t *p) {
	return (queue_t*)((ptr_t)p - offsetof(queue_t, receiver));
}

static inline thread_t *getTaskParent(const list_t *p) {
	return (thread_t*)((ptr_t)p - offsetof(thread_t, wait.waiting));
}

static inline int is_full(queue_t *q)
{
	return !(q->left);
}

static inline int is_empty(queue_t *q)
{
	return q->read == q->write;
}

static void enqueue(queue_t *q, VP_INT data)
{
	q->dtq[q->write++] = data;
	if (q->write >= q->dtqcnt)
		q->write = 0;
	q->left--;
}

static VP_INT dequeue(queue_t *q)
{
	VP_INT v = q->dtq[q->read++];

	if (q->read >= q->dtqcnt)
		q->read = 0;
	q->left++;

	return v;
}

static queue_t *get_queue(const ID dtqid)
{
	node_t *node = tree_get(&queue_tree, dtqid);

	return node? getQueueParent(node):NULL;
}

static list_t *release_sender(VP_INT *d, queue_t *q)
{
	list_t *sender = list_head(&(q->sender));

	if (sender) {
		thread_t *tp = getTaskParent(sender);

		*d = tp->wait.detail.que.data;
		list_remove(sender);
		release(tp);
	}

	return sender;
}

ER queue_initialize(void)
{
	queue_slab.unit_size = sizeof(queue_t);
	queue_slab.block_size = PAGE_SIZE;
	queue_slab.min_block = 1;
	queue_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(queue_t));
	queue_slab.palloc = palloc;
	queue_slab.pfree = pfree;
	slab_create(&queue_slab);

	tree_create(&queue_tree, &queue_slab);
	queue_hand = MIN_AUTO_ID - 1;

	return E_OK;
}

static void clear(queue_t *p, const T_CDTQ *pk_cdtq) {
	list_initialize(&(p->sender));
	list_initialize(&(p->receiver));
	p->dtqcnt = pk_cdtq->dtqcnt;
	p->dtq = pk_cdtq->dtq;

	p->read = 0;
	p->write = 0;
	p->left = p->dtqcnt;
}

ER_ID queue_create_auto(T_CDTQ *pk_cdtq)
{
	ER_ID result;

	if (!pk_cdtq)	return E_PAR;
	if (pk_cdtq->dtqcnt > MAX_DTQCNT)	return E_PAR;
/* TODO validate pk */
	if (pk_cdtq->dtqatr != TA_TFIFO)	return E_RSATR;

	enter_serialize();
	do {
		UINT *dtq;
		node_t *node;

		if (pk_cdtq->dtqcnt) {
			dtq = (UINT*)palloc(1);
			if (!dtq) {
				result =  E_NOMEM;
				break;
			}
		} else
			dtq = NULL;

		node = find_empty_key(&queue_tree, &queue_hand);
		if (!node) {
			result = E_NOID;
			if (dtq)
				pfree(dtq, 1);
			break;
		}

		pk_cdtq->dtq = dtq;
		clear(getQueueParent(node), pk_cdtq);
		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static void release_all(list_t *waiting)
{
	list_t *q;

	while ((q = list_dequeue(waiting)) != NULL) {
		thread_t *p = getTaskParent(q);

		p->wait.result = E_DLT;
		release(p);
/* TODO test */
	}
}

ER queue_destroy(ID dtqid)
{
	ER result;

	enter_serialize();
	do {
		queue_t *q = get_queue(dtqid);

		if (!q) {
			result = E_NOEXS;
			break;
		}

		release_all(&(q->receiver));
		release_all(&(q->sender));
		if (q->dtq)
			pfree(q->dtq, 1);
		tree_remove(&queue_tree, dtqid);
		result = E_OK;

	} while (FALSE);
	leave_serialize();

	thread_switch();
	return result;
}

ER queue_send(ID dtqid, VP_INT data)
{
	queue_t *q;
	list_t *receiver;

	enter_serialize();
	q = get_queue(dtqid);
	if (!q) {
		leave_serialize();
		return E_NOEXS;
	}

	receiver = list_head(&(q->receiver));
	if (receiver) {
		thread_t *tp = getTaskParent(receiver);

		tp->wait.detail.que.data = data;
		list_remove(receiver);
		release(tp);

	} else if (is_full(q)) {
		list_enqueue(&(q->sender), &(run_task->wait.waiting));
		run_task->wait.detail.que.data = data;
		run_task->wait.type = wait_que;

		leave_serialize();
		wait(run_task);
		return run_task->wait.result;

	} else
		enqueue(q, data);

	leave_serialize();
	return E_OK;
}

ER queue_receive(ID dtqid, VP_INT *p_data)
{
	queue_t *q;

	enter_serialize();
	q = get_queue(dtqid);
	if (!q) {
		leave_serialize();
		return E_NOEXS;
	}

	if (q->dtqcnt) {
		if (!is_empty(q)) {
			VP_INT d;

			*p_data = dequeue(q);
			if (release_sender(&d, q))
				enqueue(q, d);

			leave_serialize();
			return E_OK;
		}

	} else {
		VP_INT d;

		if (release_sender(&d, q)) {
			*p_data = d;
			leave_serialize();
			return E_OK;
		}
	}

	list_enqueue(&(q->receiver), &(run_task->wait.waiting));
	run_task->wait.type = wait_que;
	leave_serialize();

	wait(run_task);

	if (run_task->wait.result == E_OK)
		*p_data = run_task->wait.detail.que.data;

	return run_task->wait.result;
}
