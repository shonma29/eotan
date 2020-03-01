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
#include <nerve/ipc_utils.h>
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "ipc.h"
#include "ready.h"
#include "sync.h"
#include "thread.h"
#include "mpu/mpufunc.h"

typedef struct {
	node_t node;
	list_t caller;
	size_t max_reply;
} reply_t;

static slab_t reply_slab;
static tree_t reply_tree;
static int reply_hand;

static inline ipc_t *getPortParent(const thread_t *);
static inline reply_t *getReplyParent(const node_t *);
static void clear(ipc_t *, const T_CPOR *);


static inline ipc_t *getPortParent(const thread_t *p)
{
	return ((ipc_t *) &(p->port));
}

static inline reply_t *getReplyParent(const node_t *p)
{
	return ((reply_t *) ((uintptr_t) p - offsetof(reply_t, node)));
}

int ipc_initialize(void)
{
	create_tree(&reply_tree, &reply_slab, sizeof(reply_t), NULL);
	reply_hand = MIN_AUTO_ID - 1;
	return E_OK;
}

static void clear(ipc_t *p, const T_CPOR *pk_cpor)
{
	p->opened = true;
	list_initialize(&(p->caller));
	list_initialize(&(p->receiver));
	p->attr = pk_cpor->poratr;
	p->max_call = pk_cpor->maxcmsz;
	p->max_reply = pk_cpor->maxrmsz;
}

int ipc_open(const T_CPOR *pk_cpor)
{
	if (!pk_cpor)
		return E_PAR;
/* TODO validate pk */
	if (pk_cpor->poratr != TA_TFIFO)
		return E_RSATR;

	ER result;
	enter_serialize();
	do {
		ipc_t *port = getPortParent(running);
		if (port->opened) {
			result = E_OBJ;
			break;
/* TODO test */
		}

		clear(port, pk_cpor);
		result = E_OK;
	} while (false);
	leave_serialize();

	return result;
}

int ipc_close(void)
{
	ER result;

	enter_serialize();
	do {
		ipc_t *port = getPortParent(running);
		if (!(port->opened)) {
			result = E_OBJ;
			break;
/* TODO test */
		}

		port->opened = false;
		release_all(&(port->receiver));
/* TODO remove rendevous */
		release_all(&(port->caller));
		result = E_OK;
	} while (false);
	leave_serialize();

	dispatch();
	return result;
}

int ipc_call(const int port_id, void *message, const size_t size)
{
/* TODO validate message */
	enter_serialize();
	thread_t *th = get_thread_ptr(port_id);
	if (!th) {
		leave_serialize();
		return E_NOEXS;
	}

	ipc_t *port = getPortParent(th);
	if (!(port->opened)) {
		leave_serialize();
		return E_NOEXS;
	}

	if (size > port->max_call) {
		warn("ipc_call[%d] size %d > %d\n",
				port_id, size, port->maxcmsz);
		leave_serialize();
		return E_PAR;
	}

	list_t *q = list_head(&(port->receiver));
	if (q) {
		thread_t *tp = getThreadWaiting(q);
		if (memcpy_k2u(tp, tp->wait.detail.ipc.message, message,
				size)) {
			warn("ipc_call[%d] copy_to(%d, %p, %p, %d) error\n",
					port_id, thread_id(tp),
					tp->wait.detail.ipc.message, message,
					size);
			leave_serialize();
			return E_PAR;
		}

		list_remove(q);
		tp->wait.detail.ipc.size = size;

		int tag = tp->wait.detail.ipc.key;
		node_t *node = tree_get(&reply_tree, tag);
		tag = create_ipc_tag((int) thread_id(running), tag);
		tp->wait.detail.ipc.key = tag;

		reply_t *r = getReplyParent(node);
		list_enqueue(&(r->caller), &(running->wait.waiting));
		running->wait.detail.ipc.message = message;
		running->wait.detail.ipc.key = tag;
		running->wait.type = wait_reply;
		release(tp);
	} else {
		list_enqueue(&(port->caller), &(running->wait.waiting));
		running->wait.detail.ipc.size = size;
		running->wait.detail.ipc.message = message;
		running->wait.type = wait_call;
/* TODO test */
	}

	wait(running);
	return ((running->wait.result) ?
			running->wait.result : running->wait.detail.ipc.size);
}

int ipc_receive(const int port_id, int *tag, void *message)
{
/* TODO validate message */
	enter_serialize();
	thread_t *th = get_thread_ptr(port_id);
	if (!th) {
		leave_serialize();
		return E_NOEXS;
	}

	ipc_t *port = getPortParent(th);
	node_t *node = slab_alloc(&reply_slab);
	if (!node) {
		leave_serialize();
		return E_NOMEM;
	}

	if (!find_empty_key(&reply_tree, &reply_hand, node)) {
		slab_free(&reply_slab, node);
		leave_serialize();
		return E_NOID;
/* TODO test */
	}

	int reply_key = node->key;
	reply_t *r = getReplyParent(node);
	list_initialize(&(r->caller));
	r->max_reply = port->max_reply;

	ER_UINT result;
	list_t *q = list_head(&(port->caller));
	if (q) {
		thread_t *tp = getThreadWaiting(q);
		result = tp->wait.detail.ipc.size;

		if (memcpy_u2k(tp, message, tp->wait.detail.ipc.message,
				result)) {
			warn("ipc_receive[%d] copy_from(%d, %p, %p, %d) error\n",
					port_id, thread_id(tp), message,
					tp->wait.detail.ipc.message, result);
			node = tree_remove(&reply_tree, reply_key);
			if (node)
				slab_free(&reply_slab, node);

			leave_serialize();
			return E_PAR;
/* TODO test */
		}

		list_remove(q);
		list_enqueue(&(r->caller), &(tp->wait.waiting));
		*tag = tp->wait.detail.ipc.key =
				create_ipc_tag((int) thread_id(tp), reply_key);
		tp->wait.type = wait_reply;
/* TODO test */
	} else {
		list_enqueue(&(port->receiver), &(running->wait.waiting));
		running->wait.detail.ipc.message = message;
		running->wait.detail.ipc.key = reply_key;
		running->wait.type = wait_call;
		wait(running);

		enter_serialize();
		if (running->wait.result) {
			result = running->wait.result;
			node = tree_remove(&reply_tree, reply_key);
			if (node)
				slab_free(&reply_slab, node);
		} else {
			result = running->wait.detail.ipc.size;
			*tag = running->wait.detail.ipc.key;
		}
/* TODO test */
	}

	leave_serialize();
	return result;
}

int ipc_send(const int tag, const void *message, const size_t size)
{
	int reply_key = key_of_ipc(tag);
/* TODO validate message */
	enter_serialize();

	node_t *node = tree_get(&reply_tree, reply_key);
	if (!node) {
		node = tree_remove(&reply_tree, reply_key);
		if (node)
			slab_free(&reply_slab, node);

		leave_serialize();
		return E_OBJ;
	}

	reply_t *r = getReplyParent(node);
	if (size > r->max_reply) {
		warn("ipc_send[%d] size %d > %d\n",
				reply_key, size, r->maxrmsz);
		node = tree_remove(&reply_tree, reply_key);
		if (node)
			slab_free(&reply_slab, node);

		leave_serialize();
		return E_PAR;
	}

	ER result;
	list_t *q = list_head(&(r->caller));
	if (q) {
		thread_t *tp = getThreadWaiting(q);
		if (memcpy_k2u(tp, tp->wait.detail.ipc.message, message,
				size)) {
			warn("ipc_send[%d] copy_to(%d, %p, %p, %d) error\n",
					reply_key, thread_id(tp),
					tp->wait.detail.ipc.message, message,
					size);
			node = tree_remove(&reply_tree, reply_key);
			if (node)
				slab_free(&reply_slab, node);

			leave_serialize();
			return E_PAR;
		}

		list_remove(q);
		tp->wait.detail.ipc.size = size;
		release(tp);
		result = E_OK;
	} else
		result = E_OBJ;
/* TODO test */

	node = tree_remove(&reply_tree, reply_key);
	if (node)
		slab_free(&reply_slab, node);

	leave_serialize();
	dispatch();
	return result;
}
