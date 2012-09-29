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
#include "../include/itron/types.h"
#include "../include/itron/rendezvous.h"
#include "../include/set/list.h"
#include "../include/set/tree.h"
#include "mpu.h"
#include "func.h"
#include "sync.h"
#include "setting.h"
#include "task.h"

typedef struct {
	node_t node;
	list_t caller;
	list_t acceptor;
	ATR poratr;
	UINT maxcmsz;
	UINT maxrmsz;
} port_t;

typedef struct {
	node_t node;
	list_t caller;
	UINT maxrmsz;
} rendezvous_t;

static slab_t port_slab;
static tree_t port_tree;
static int port_hand;
static slab_t rdv_slab;
static tree_t rdv_tree;
static int rdv_hand;

static inline port_t *getPortParent(const node_t *p);
static inline port_t *getCallerParent(const list_t *p);
static inline port_t *getAcceptorParent(const list_t *p);
static inline rendezvous_t *getRdvParent(const node_t *p);
static inline T_TCB *getTaskParent(const list_t *p);
static void clear(port_t *p, const T_CPOR *pk_cpor);
static node_t *find_empty_key(tree_t *tree, int *hand);
static void release_all(list_t *waiting);


static inline port_t *getPortParent(const node_t *p) {
	return (port_t*)((ptr_t)p - offsetof(port_t, node));
}

static inline port_t *getCallerParent(const list_t *p) {
	return (port_t*)((ptr_t)p - offsetof(port_t, caller));
}

static inline port_t *getAcceptorParent(const list_t *p) {
	return (port_t*)((ptr_t)p - offsetof(port_t, acceptor));
}

static inline rendezvous_t *getRdvParent(const node_t *p) {
	return (rendezvous_t*)((ptr_t)p - offsetof(rendezvous_t, node));
}

static inline T_TCB *getTaskParent(const list_t *p) {
	return (T_TCB*)((ptr_t)p - offsetof(T_TCB, wait.waiting));
}

ER port_initialize(void)
{
	port_slab.unit_size = sizeof(port_t);
	port_slab.block_size = I386_PAGESIZE;
	port_slab.min_block = 1;
	port_slab.max_block = tree_max_block(65536, I386_PAGESIZE,
			sizeof(port_t));
	port_slab.palloc = palloc;
	port_slab.pfree = pfree;
	slab_create(&port_slab);

	tree_create(&port_tree, &port_slab);
	port_hand = MIN_AUTO_ID - 1;

	rdv_slab.unit_size = sizeof(rendezvous_t);
	rdv_slab.block_size = I386_PAGESIZE;
	rdv_slab.min_block = 1;
	rdv_slab.max_block = tree_max_block(65536, I386_PAGESIZE,
			sizeof(rendezvous_t));
	rdv_slab.palloc = palloc;
	rdv_slab.pfree = pfree;
	slab_create(&rdv_slab);

	tree_create(&rdv_tree, &rdv_slab);
	rdv_hand = MIN_AUTO_ID - 1;

	return E_OK;
}

static void clear(port_t *p, const T_CPOR *pk_cpor) {
	list_initialize(&(p->caller));
	list_initialize(&(p->acceptor));
	p->poratr = pk_cpor->poratr;
	p->maxcmsz = pk_cpor->maxcmsz;
	p->maxrmsz = pk_cpor->maxrmsz;
}

ER port_create(ID porid, T_CPOR *pk_cpor)
{
	ER result;

	if ((porid < MIN_MANUAL_ID)
			|| (porid > MAX_MANUAL_ID)) {
		return E_ID;
	}

	if (!pk_cpor)	return E_PAR;

	if (pk_cpor->poratr != TA_TFIFO)	return E_RSATR;

	enter_serialize();
	do {
		node_t *node;

		if (tree_get(&port_tree, porid)) {
			result = E_OBJ;
			break;
		}

		node = tree_put(&port_tree, porid);
		if (!node) {
			result = E_NOMEM;
			break;
/* TODO test */
		}

		clear(getPortParent(node), pk_cpor);
		result = E_OK;

	} while (FALSE);
	leave_serialize();

	return result;
}

static node_t *find_empty_key(tree_t *tree, int *hand) {
	int key;

	for (key = *hand + 1; key <= MAX_AUTO_ID; key++) {
/*int max = (tree == &(port_tree))? 49156:MAX_AUTO_ID;
	for (key = *hand + 1; key <= max; key++) {*/
		if (!tree_get(tree, key)) {
			*hand = key;
			return tree_put(tree, key);
		}
	}
/* TODO test */
	for (key = MIN_AUTO_ID; key < *hand; key++) {
		if (!tree_get(tree, key)) {
			*hand = key;
			return tree_put(tree, key);
		}
	}

	return NULL;
}

ER_ID port_create_auto(T_CPOR *pk_cpor)
{
	ER_ID result;

	if (!pk_cpor)	return E_PAR;

	if (pk_cpor->poratr != TA_TFIFO)	return E_RSATR;

	enter_serialize();
	do {
		node_t *node = find_empty_key(&port_tree, &port_hand);

		if (!node) {
			result = E_NOID;
			break;
		}

		clear(getPortParent(node), pk_cpor);
		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static void release_all(list_t *waiting) {
	list_t *q;

	while ((q = list_dequeue(waiting)) != NULL) {
		T_TCB *p = getTaskParent(q);

		p->wait.result = E_DLT;
		release(p);
/* TODO test */
	}
}

ER port_destroy(ID porid)
{
	ER result;

	enter_serialize();
	do {
		port_t *p;
		node_t *node = tree_get(&port_tree, porid);

		if (!node) {
			result = E_NOEXS;
			break;
/* TODO test */
		}

		p = getPortParent(node);
		release_all(&(p->acceptor));
/* TODO remove rendevous */
		release_all(&(p->caller));
		tree_remove(&port_tree, porid);
		result = E_OK;
/* TODO test */

	} while (FALSE);
	leave_serialize();

	task_switch();
	return result;
}

ER_UINT port_call(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz)
{
	port_t *p;
	node_t *node;
	list_t *q;

	enter_serialize();
	node = tree_get(&port_tree, porid);
	if (!node) {
		leave_serialize();
		return E_NOEXS;
	}

	p = getPortParent(node);

	if (cmsgsz > p->maxcmsz) {
		leave_serialize();
		return E_PAR;
	}

	q = list_dequeue(&(p->acceptor));
	if (q) {
		T_TCB *tp = getTaskParent(q);
		rendezvous_t *r;
		if (vput_reg(tp->tskid, tp->wait.detail.por.msg, cmsgsz, msg)) {
			leave_serialize();
			return E_PAR;
/* TODO test */
		}

		tp->wait.detail.por.size = cmsgsz;

		node = tree_get(&rdv_tree, tp->wait.detail.por.rdvno);
		r = getRdvParent(node);

		list_enqueue(&(r->caller), &(run_task->wait.waiting));
		run_task->wait.detail.por.msg = msg;
		run_task->wait.detail.por.rdvno = tp->wait.detail.por.rdvno;
		run_task->wait.type = wait_rdv;
		release(tp);
	}
	else {
		list_enqueue(&(p->caller), &(run_task->wait.waiting));
		run_task->wait.detail.por.size = cmsgsz;
		run_task->wait.detail.por.msg = msg;
		run_task->wait.type = wait_por;
/* TODO test */
	}

	leave_serialize();

	wait(run_task);
	return (run_task->wait.result)?
			run_task->wait.result:run_task->wait.detail.por.size;
}

ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg)
{
	port_t *p;
	node_t *node;
	rendezvous_t *r;
	list_t *q;
	ER_UINT result;
	int rdvno;

	enter_serialize();
	node = tree_get(&port_tree, porid);
	if (!node) {
		leave_serialize();
		return E_NOEXS;
/* TODO test */
	}

	p = getPortParent(node);

	node = find_empty_key(&rdv_tree, &rdv_hand);
	if (!node) {
		leave_serialize();
		return E_NOID;
/* TODO test */
	}

	rdvno = node->key;
	r = getRdvParent(node);
	list_initialize(&(r->caller));
	r->maxrmsz = p->maxrmsz;
	*p_rdvno = rdvno;

	q = list_dequeue(&(p->caller));
	if (q) {
		T_TCB *tp = getTaskParent(q);

		result = tp->wait.detail.por.size;

		if (vget_reg(tp->tskid, tp->wait.detail.por.msg, result, msg)) {
			leave_serialize();
			return E_PAR;
/* TODO test */
		}

		list_enqueue(&(r->caller), &(tp->wait.waiting));
		tp->wait.detail.por.rdvno = rdvno;
		tp->wait.type = wait_rdv;
	}
	else {
		list_enqueue(&(p->acceptor), &(run_task->wait.waiting));
		run_task->wait.detail.por.msg = msg;
		run_task->wait.detail.por.rdvno = rdvno;
		run_task->wait.type = wait_por;
		leave_serialize();

		wait(run_task);

		enter_serialize();
		if (run_task->wait.result) {
			result = run_task->wait.result;
			tree_remove(&rdv_tree, rdvno);
		}
		else	result = run_task->wait.detail.por.size;
/* TODO test */
	}

	leave_serialize();
	return result;
}

ER port_reply(RDVNO rdvno, VP msg, UINT rmsgsz)
{
	rendezvous_t *r;
	node_t *node;
	list_t *q;
	ER result;

	enter_serialize();
	node = tree_get(&rdv_tree, rdvno);
	if (!node) {
		leave_serialize();
		return E_OBJ;
/* TODO test */
	}

	r = getRdvParent(node);

	if (rmsgsz > r->maxrmsz) {
		printk("[KERN] port_reply[%d] rmsgsiz %d > %d\n",
				rdvno, rmsgsz, r->maxrmsz);
/* TODO tree_remove? */
		leave_serialize();
		return E_PAR;
/* TODO test */
	}

	q = list_dequeue(&(r->caller));
	if (q) {
		T_TCB *tp = getTaskParent(q);

		if (vput_reg(tp->tskid, tp->wait.detail.por.msg,
				rmsgsz, msg)) {
			printk("[KERN] port_reply[%d] vput_reg(%d, %p, %d, %p) error\n",
					rdvno, tp->tskid,
					tp->wait.detail.por.msg, rmsgsz, msg);
/* TODO tree_remove? */
			leave_serialize();
			return E_PAR;
/* TODO test */
		}

		tp->wait.detail.por.size = rmsgsz;
		release(tp);
		result = E_OK;
	}
	else	result = E_OBJ;
/* TODO test */

	tree_remove(&rdv_tree, rdvno);
	leave_serialize();

	task_switch();
	return result;
}
