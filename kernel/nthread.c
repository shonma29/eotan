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
#include <vm.h>
#include <set/list.h>
#include <set/tree.h>
#include "func.h"
#include "ready.h"
#include "setting.h"
#include "sync.h"
#include "thread.h"
#include "mpu/mpu.h"
#include "mpu/mpufunc.h"

static slab_t thread_slab;
static tree_t thread_tree;
static int thread_hand;

static inline T_TCB *getThreadParent(const node_t *p);
static ER setup(T_TCB *th, T_CTSK *pk_ctsk, int tskid);
static void release_resources(T_TCB *th);

static inline T_TCB *getThreadParent(const node_t *p) {
	return (T_TCB*)((ptr_t)p - offsetof(T_TCB, node));
}

ER thread_initialize(void)
{
	thread_slab.unit_size = sizeof(T_TCB);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(T_TCB));
	thread_slab.palloc = palloc;
	thread_slab.pfree = pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, &thread_slab);
	thread_hand = MIN_AUTO_ID - 1;

	ready_initialize();

	return E_OK;
}

T_TCB *get_thread_ptr(ID tskid)
{
	node_t *node = tree_get(&thread_tree, tskid);

	return node? getThreadParent(node):NULL;
}

ER_ID thread_initialize1(void)
{
	ER_ID result;

	enter_serialize();
	do {
		T_TCB *th;
		node_t *node = find_empty_key(&thread_tree, &thread_hand);
		if (!node) {
			result = E_NOID;
			break;
		}

		th = getThreadParent(node);
		memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

		list_initialize(&(th->queue));
		th->tskid = node->key;
		th->domain_id = KERNEL_DOMAIN_ID;
		th->tsklevel = th->tsklevel0 = MAX_PRIORITY;
		th->tskstat = TTS_RUN;
		list_initialize(&(th->wait.waiting));

		set_page_table(th, (VP)PAGE_DIR_ADDR);

		run_task = th;
		ready_enqueue(th->tsklevel, &(th->queue));
		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static ER setup(T_TCB *th, T_CTSK *pk_ctsk, int tskid)
{
	ER result;
	W i;

	memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

	list_initialize(&(th->queue));
	th->tskid = tskid;
	th->domain_id = pk_ctsk->domain_id;
	th->tsklevel = th->tsklevel0 = pk_ctsk->itskpri;
	th->tskstat = TTS_DMT;
	list_initialize(&(th->wait.waiting));
	th->exinf = pk_ctsk->exinf;

	result = create_context(th, pk_ctsk);
	if (result != E_OK)
		return result;

	set_page_table(th, (th->domain_id == KERNEL_DOMAIN_ID)?
			(VP)PAGE_DIR_ADDR
			:kern_v2p(dup_vmap_table((ADDR_MAP)MPU_PAGE_TABLE(run_task))));

	for (i = 0; i < MAX_REGION; i++)
		th->regions[i].permission = 0;

	th->regions[0].start_addr = (VP)MIN_KERNEL;
	th->regions[0].min_size = KERNEL_SIZE;
	th->regions[0].max_size = KERNEL_SIZE;
	th->regions[0].permission = VM_READ | VM_WRITE;

	return E_OK;
}

ER_ID thread_create_auto(T_CTSK *pk_ctsk)
{
	ER_ID result;

	if (pk_ctsk->tskatr & TA_ASM)
		return E_RSATR;

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
		} else
			result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static void release_resources(T_TCB *th)
{
	if (th->domain_id != KERNEL_DOMAIN_ID)
		release_vmap((ADDR_MAP)MPU_PAGE_TABLE(th));

	pfree((VP)kern_v2p(th->stackptr0), pages(th->stksz0));
}

ER thread_destroy(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		T_TCB *th = get_thread_ptr(tskid);

		if (!th) {
			result = E_NOEXS;
			break;
		}

		if (th->tskstat != TTS_DMT)
			return E_OBJ;

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
		T_TCB *th;

		if (tskid == TSK_SELF)
			tskid = run_task->tskid;

		th = get_thread_ptr(tskid);
		if (!th) {
			result = E_NOEXS;
			break;
		}

		if (th->tskstat != TTS_DMT) {
			result = E_QOVR;
			break;
		}

		th->tskstat = TTS_RDY;
		th->total = 0;
		th->quantum = QUANTUM;
//TODO fix context_create_kernel
//		set_arg(th, th->exinf);

		ready_enqueue(th->tsklevel, &(th->queue));
		result = E_OK;
		leave_serialize();
		thread_switch();

	} while (FALSE);
	leave_serialize();

	return result;
}

void thread_end(void)
{
	enter_serialize();
	list_remove(&(run_task->queue));
	run_task->tskstat = TTS_DMT;
	leave_serialize();

	thread_switch();
}

void thread_end_and_destroy(void)
{
	T_TCB *th;

	enter_serialize();
	th = run_task;

	tree_remove(&thread_tree, th->tskid);
	list_remove(&(th->queue));

	if (th->domain_id != KERNEL_DOMAIN_ID)
		context_reset_page_table();

	release_resources(th);
	leave_serialize();

	thread_switch();
}

ER thread_terminate(ID tskid)
{
	ER result;

	enter_serialize();
	do {
		T_TCB *th = get_thread_ptr(tskid);

		if (!th) {
			result = E_NOEXS;
			break;
		}

		switch (th->tskstat) {
		case TTS_RUN:
			result = E_ILUSE;
			break;

		case TTS_RDY:
			list_remove(&(th->queue));
			th->tskstat = TTS_DMT;
			result = E_OK;
			break;

		case TTS_WAI:
		case TTS_WAS:
			if (th->wait.type)
				list_remove(&(th->wait.waiting));
		case TTS_SUS:
			th->tskstat = TTS_DMT;
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
