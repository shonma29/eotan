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
#include <string.h>
#include <nerve/config.h>
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "ready.h"
#include "sync.h"
#include "thread.h"
#include "mpu/mpufunc.h"

static slab_t thread_slab;
static tree_t thread_tree;
static int thread_hand;

static inline thread_t *getThreadParent(const node_t *p);
static ER_ID idle_initialize(void);
static ER setup(thread_t *th, T_CTSK *pk_ctsk, int tskid);
static void release_resources(thread_t *th);

static inline thread_t *getThreadParent(const node_t *p) {
	return (thread_t*)((intptr_t)p - offsetof(thread_t, node));
}

ER thread_initialize(void)
{
	create_tree(&thread_tree, &thread_slab, sizeof(thread_t), NULL);
	thread_hand = MIN_AUTO_ID - 1;

	ready_initialize();
	idle_initialize();

	return E_OK;
}

thread_t *get_thread_ptr(ID tskid)
{
	node_t *node = tree_get(&thread_tree, tskid);

	return node? getThreadParent(node):NULL;
}

static ER_ID idle_initialize(void)
{
	ER_ID result;

	enter_serialize();
	do {
		thread_t *th;
		node_t *node;

		if (tree_get(&thread_tree, TSK_NONE)) {
			result = E_OBJ;
			break;
		}

		node = tree_put(&thread_tree, TSK_NONE);
		if (!node) {
			result = E_NOMEM;
			break;
		}

		th = getThreadParent(node);
		memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

		list_initialize(&(th->queue));
		th->status = TTS_RUN;
		list_initialize(&(th->wait.waiting));
		list_initialize(&(th->locking));
		//th->time.total = 0;
		//th->time.left = TIME_QUANTUM;
		//th->wakeup_count = 0;

		th->attr.page_table = NULL;
		th->priority = th->attr.priority = MAX_PRIORITY;
		//th->attr.arg = NULL;
		//th->attr.kstack_tail = (void*)CORE_STACK_ADDR;
		//th->attr.entry = kern_start;

		/* kthread don't need to set tss */
		//context_set_kernel_sp(&(th->mpu), th->attr.kstack_tail);
		context_set_page_table(&(th->mpu), (VP)KTHREAD_DIR_ADDR);

		running = th;
		ready_enqueue(th->priority, &(th->queue));
		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static ER setup(thread_t *th, T_CTSK *pk_ctsk, int tskid)
{
	void *p = palloc();

	if (!p)
		return E_NOMEM;

	memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

	list_initialize(&(th->queue));
	th->status = TTS_DMT;
	list_initialize(&(th->wait.waiting));
	list_initialize(&(th->locking));
	//th->wakeup_count = 0;

	th->attr.page_table = pk_ctsk->page_table;
	th->attr.priority = pk_ctsk->itskpri;
	th->attr.arg = pk_ctsk->exinf;
	th->attr.kstack_tail = (char*)kern_p2v(p) + pk_ctsk->stksz;
	th->attr.ustack_top = pk_ctsk->ustack_top;
	th->attr.entry = pk_ctsk->task;

	context_set_kernel_sp(&(th->mpu), th->attr.kstack_tail);
	context_set_page_table(&(th->mpu),
			is_kthread(th)?
					(VP)KTHREAD_DIR_ADDR
					//TODO null check
					:th->attr.page_table);

	return E_OK;
}

ER_ID thread_create_auto(T_CTSK *pk_ctsk)
{
	ER_ID result;

	if (pk_ctsk->tskatr & TA_ASM)
		return E_RSATR;

	if (pk_ctsk->stksz != KTHREAD_STACK_SIZE)
		return E_PAR;

	enter_serialize();
	do {
		node_t *node = find_empty_key(&thread_tree, &thread_hand);

		if (!node) {
			result = E_NOID;
			break;
		}

		result = setup(getThreadParent(node), pk_ctsk, node->key);
		if (result) {
			tree_remove(&thread_tree, node->key);
			break;
		}

		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static void release_resources(thread_t *th)
{
	pfree((VP)kern_v2p((char*)(th->attr.kstack_tail)) - KTHREAD_STACK_SIZE);
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

		tree_remove(&thread_tree, tskid);
		release_resources(th);
		result = E_OK;

	} while (FALSE);
	leave_serialize();

	return result;
}

ER thread_start(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		thread_t *th;

		if (tskid == TSK_SELF)
			tskid = thread_id(running);

		th = get_thread_ptr(tskid);
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

		th->time.total = 0;
		th->time.left = TIME_QUANTUM;
		th->priority = th->attr.priority;
		th->wakeup_count = 0;
		create_context(th);
		result = E_OK;

		th->status = TTS_RDY;
		ready_enqueue(th->priority, &(th->queue));
		leave_serialize();
		dispatch();

	} while (FALSE);

	return result;
}

void thread_end(void)
{
	enter_serialize();
	list_remove(&(running->queue));
	running->status = TTS_DMT;
	mutex_unlock_all(running);
	leave_serialize();

	dispatch();
}

void thread_end_and_destroy(void)
{
	thread_t *th;

	enter_serialize();
	th = running;

	tree_remove(&thread_tree, thread_id(th));
	list_remove(&(th->queue));
	mutex_unlock_all(th);

	if (!is_kthread(th))
		context_reset_page_table();

	release_resources(th);
	leave_serialize();

	dispatch();
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
			mutex_unlock_all(th);
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
			mutex_unlock_all(th);
			result = E_OK;
			break;

		default:
			result = E_OBJ;
			break;
		}
	} while (FALSE);
	leave_serialize();

	return result;
}

void thread_tick(void)
{
	running->time.total++;

	if (!is_kthread(running))
		if (!(--(running->time.left))) {
			running->time.left = TIME_QUANTUM;
			ready_rotate(running->priority);
		}
}

ER thread_sleep(void)
{
	enter_serialize();

	if (running->wakeup_count > 0) {
		running->wakeup_count--;
		leave_serialize();
		return E_OK;

	} else {
		running->wait.type = wait_slp;
		leave_serialize();
		wait(running);
		return running->wait.result;
	}
}

ER thread_wakeup(ID tskid)
{
	ER result;

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
		case TTS_DMT:
			result = E_OBJ;
			break;

		case TTS_WAI:
		case TTS_WAS:
			if (th->wait.type == wait_slp) {
				release(th);
				result = E_OK;
				break;
			}

		default:
			if (th->wakeup_count >= MAX_WAKEUP_COUNT)
				result = E_QOVR;

			else {
				th->wakeup_count++;
				result = E_OK;
			}

			break;
		}

	} while (FALSE);
	leave_serialize();

	return result;
}

ID thread_get_id(void)
{
	return running->node.key;
}
