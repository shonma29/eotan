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
#include <core/options.h>
#include <nerve/config.h>
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "ready.h"
#include "rendezvous.h"
#include "sync.h"
#include "thread.h"
#include "mpu/mpufunc.h"

typedef struct {
	node_t node;
	list_t caller;
	UINT maxrmsz;
} rendezvous_t;

static slab_t rdv_slab;
static tree_t rdv_tree;
static int rdv_hand;

static inline port_t *getPortParent(const thread_t *p);
static inline rendezvous_t *getRdvParent(const node_t *p);
static void clear(port_t *p, const T_CPOR *pk_cpor);


static inline port_t *getPortParent(const thread_t *p) {
	return (port_t*)&(p->port);
}

static inline rendezvous_t *getRdvParent(const node_t *p) {
	return (rendezvous_t*)((intptr_t)p - offsetof(rendezvous_t, node));
}

ER port_initialize(void)
{
	create_tree(&rdv_tree, &rdv_slab, sizeof(rendezvous_t), NULL);
	rdv_hand = MIN_AUTO_ID - 1;

	return E_OK;
}

static void clear(port_t *p, const T_CPOR *pk_cpor) {
	p->opened = true;
	list_initialize(&(p->caller));
	list_initialize(&(p->acceptor));
	p->poratr = pk_cpor->poratr;
	p->maxcmsz = pk_cpor->maxcmsz;
	p->maxrmsz = pk_cpor->maxrmsz;
}

ER port_open(T_CPOR *pk_cpor)
{
	ER result;

	if (!pk_cpor)
		return E_PAR;
/* TODO validate pk */
	if (pk_cpor->poratr != TA_TFIFO)
		return E_RSATR;

	enter_serialize();
	do {
		port_t *port = getPortParent(running);

		if (port->opened) {
			result = E_OBJ;
			break;
/* TODO test */
		}

		clear(port, pk_cpor);
		result = E_OK;

	} while (FALSE);
	leave_serialize();

	return result;
}

ER port_close(void)
{
	ER result;

	enter_serialize();
	do {
		port_t *port = getPortParent(running);

		if (!(port->opened)) {
			result = E_OBJ;
			break;
/* TODO test */
		}

		port->opened = false;
		release_all(&(port->acceptor));
/* TODO remove rendevous */
		release_all(&(port->caller));
		result = E_OK;

	} while (FALSE);
	leave_serialize();

	dispatch();
	return result;
}

ER_UINT port_call(ID porid, VP msg, UINT cmsgsz)
{
	thread_t *th;
	port_t *port;
	list_t *q;
/* TODO validate msg */
	enter_serialize();
	th = get_thread_ptr(porid);
	if (!th) {
		leave_serialize();
		return E_NOEXS;
	}

	port = getPortParent(th);
	if (!(port->opened)) {
		leave_serialize();
		return E_NOEXS;
	}

	if (cmsgsz > port->maxcmsz) {
		printk("port_call[%d] cmsgsz %d > %d\n",
				porid, cmsgsz, port->maxcmsz);
		leave_serialize();
		return E_PAR;
	}

	q = list_head(&(port->acceptor));
	if (q) {
		thread_t *tp = getThreadWaiting(q);
		node_t *node;
		rendezvous_t *r;
		int rdvno;

		if (memcpy_k2u(tp, tp->wait.detail.por.msg, msg, cmsgsz)) {
			printk("port_call[%d] copy_to(%d, %p, %p, %d) error\n",
					porid, thread_id(tp),
					tp->wait.detail.por.msg, msg, cmsgsz);
			leave_serialize();
			return E_PAR;
		}

		list_remove(q);
		tp->wait.detail.por.size = cmsgsz;

		rdvno = tp->wait.detail.por.rdvno;
		node = tree_get(&rdv_tree, rdvno);
		rdvno = get_rdvno(thread_id(running), rdvno);
		tp->wait.detail.por.rdvno = rdvno;
		r = getRdvParent(node);

		list_enqueue(&(r->caller), &(running->wait.waiting));
		running->wait.detail.por.msg = msg;
		running->wait.detail.por.rdvno = rdvno;
		running->wait.type = wait_rdv;
		release(tp);
	}
	else {
		list_enqueue(&(port->caller), &(running->wait.waiting));
		running->wait.detail.por.size = cmsgsz;
		running->wait.detail.por.msg = msg;
		running->wait.type = wait_por;
/* TODO test */
	}

	leave_serialize();

	wait(running);
	return (running->wait.result)?
			running->wait.result:running->wait.detail.por.size;
}

ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg)
{
	thread_t *th;
	port_t *port;
	node_t *node;
	rendezvous_t *r;
	list_t *q;
	ER_UINT result;
	int rdvno;
/* TODO validate msg */
	enter_serialize();
	th = get_thread_ptr(porid);
	if (!th) {
		leave_serialize();
		return E_NOEXS;
	}

	port = getPortParent(th);

	node = slab_alloc(&rdv_slab);
	if (!node) {
		leave_serialize();
		return E_NOMEM;
	}

	if (!find_empty_key(&rdv_tree, &rdv_hand, node)) {
		slab_free(&rdv_slab, node);
		leave_serialize();
		return E_NOID;
/* TODO test */
	}

	rdvno = node->key;
	r = getRdvParent(node);
	list_initialize(&(r->caller));
	r->maxrmsz = port->maxrmsz;

	q = list_head(&(port->caller));
	if (q) {
		thread_t *tp = getThreadWaiting(q);

		result = tp->wait.detail.por.size;

		if (memcpy_u2k(tp, msg, tp->wait.detail.por.msg, result)) {
			printk("port_accept[%d] copy_from(%d, %p, %p, %d) error\n",
					porid, thread_id(tp), msg,
					tp->wait.detail.por.msg, result);
			node = tree_remove(&rdv_tree, rdvno);
			if (node)
				slab_free(&rdv_slab, node);
			leave_serialize();
			return E_PAR;
/* TODO test */
		}

		list_remove(q);
		list_enqueue(&(r->caller), &(tp->wait.waiting));
		*p_rdvno = tp->wait.detail.por.rdvno = get_rdvno(thread_id(tp), rdvno);
		tp->wait.type = wait_rdv;
/* TODO test */
	}
	else {
		list_enqueue(&(port->acceptor), &(running->wait.waiting));
		running->wait.detail.por.msg = msg;
		running->wait.detail.por.rdvno = rdvno;
		running->wait.type = wait_por;
		leave_serialize();

		wait(running);

		enter_serialize();
		if (running->wait.result) {
			result = running->wait.result;
			node = tree_remove(&rdv_tree, rdvno);
			if (node)
				slab_free(&rdv_slab, node);
		} else {
			result = running->wait.detail.por.size;
			*p_rdvno = running->wait.detail.por.rdvno;
		}
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

/* TODO validate msg */
	enter_serialize();
	rdvno = get_rdv_seq(rdvno);
	node = tree_get(&rdv_tree, rdvno);
	if (!node) {
		leave_serialize();
		node = tree_remove(&rdv_tree, rdvno);
		if (node)
			slab_free(&rdv_slab, node);
		return E_OBJ;
	}

	r = getRdvParent(node);

	if (rmsgsz > r->maxrmsz) {
		printk("port_reply[%d] rmsgsz %d > %d\n",
				rdvno, rmsgsz, r->maxrmsz);
		leave_serialize();
		node = tree_remove(&rdv_tree, rdvno);
		if (node)
			slab_free(&rdv_slab, node);
		return E_PAR;
	}

	q = list_head(&(r->caller));
	if (q) {
		thread_t *tp = getThreadWaiting(q);

		if (memcpy_k2u(tp, tp->wait.detail.por.msg, msg, rmsgsz)) {
			printk("port_reply[%d] copy_to(%d, %p, %p, %d) error\n",
					rdvno, thread_id(tp),
					tp->wait.detail.por.msg, msg, rmsgsz);
			leave_serialize();
			node = tree_remove(&rdv_tree, rdvno);
			if (node)
				slab_free(&rdv_slab, node);
			return E_PAR;
		}

		list_remove(q);
		tp->wait.detail.por.size = rmsgsz;
		release(tp);
		result = E_OK;
	}
	else	result = E_OBJ;
/* TODO test */

	node = tree_remove(&rdv_tree, rdvno);
	if (node)
		slab_free(&rdv_slab, node);
	leave_serialize();

	dispatch();
	return result;
}
