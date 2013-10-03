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
#include <mpu/config.h>
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

static inline thread_t *getThreadParent(const node_t *p);
static ER setup(thread_t *th, T_CTSK *pk_ctsk, int tskid);
static void release_resources(thread_t *th);

static inline thread_t *getThreadParent(const node_t *p) {
	return (thread_t*)((ptr_t)p - offsetof(thread_t, node));
}

ER thread_initialize(void)
{
	thread_slab.unit_size = sizeof(thread_t);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(thread_t));
	thread_slab.palloc = palloc;
	thread_slab.pfree = pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, &thread_slab);
	thread_hand = MIN_AUTO_ID - 1;

	ready_initialize();

	return E_OK;
}

thread_t *get_thread_ptr(ID tskid)
{
	node_t *node = tree_get(&thread_tree, tskid);

	return node? getThreadParent(node):NULL;
}

ER_ID idle_initialize(void)
{
	ER_ID result;

	enter_serialize();
	do {
		thread_t *th;
		node_t *node = find_empty_key(&thread_tree, &thread_hand);
		if (!node) {
			result = E_NOID;
			break;
		}

		th = getThreadParent(node);
		memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

		list_initialize(&(th->queue));
		th->attr.domain_id = KERNEL_DOMAIN_ID;
		th->priority = th->attr.priority = MAX_PRIORITY;
		th->status = TTS_RUN;
		list_initialize(&(th->wait.waiting));

		set_page_table(th, (VP)PAGE_DIR_ADDR);

		run_task = th;
		ready_enqueue(th->priority, &(th->queue));
		result = node->key;

	} while (FALSE);
	leave_serialize();

	return result;
}

static ER setup(thread_t *th, T_CTSK *pk_ctsk, int tskid)
{
	W i;
	void *p = palloc(1);

	if (!p)
		return E_NOMEM;

	memset(&(th->queue), 0, sizeof(*th) - sizeof(node_t));

	list_initialize(&(th->queue));
	th->attr.domain_id = pk_ctsk->domain_id;
	th->attr.priority = pk_ctsk->itskpri;
	th->status = TTS_DMT;
	list_initialize(&(th->wait.waiting));
	th->attr.arg = pk_ctsk->exinf;
	th->attr.kstack.addr = kern_p2v(p);
	th->attr.kstack.length = KERNEL_STACK_SIZE;
	th->attr.kstack_top = th->attr.kstack.addr + pk_ctsk->stksz;
	th->attr.entry = pk_ctsk->task;

	create_context(th);
	set_page_table(th, (th->attr.domain_id == KERNEL_DOMAIN_ID)?
			(VP)PAGE_DIR_ADDR
//TODO create user page table in mm
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

	if (pk_ctsk->stksz > KERNEL_STACK_SIZE)
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
	if (th->attr.domain_id != KERNEL_DOMAIN_ID)
		release_vmap((ADDR_MAP)MPU_PAGE_TABLE(th));

	pfree((VP)kern_v2p(th->attr.kstack.addr), pages(th->attr.kstack.length));
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

		if (th->status != TTS_DMT)
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
		thread_t *th;

		if (tskid == TSK_SELF)
			tskid = thread_id(run_task);

		th = get_thread_ptr(tskid);
		if (!th) {
			result = E_NOEXS;
			break;
		}

		if (th->status != TTS_DMT) {
			result = E_QOVR;
			break;
		}

		th->status = TTS_RDY;
		th->time.total = 0;
		th->time.left = TIME_QUANTUM;
		th->priority = th->attr.priority;
//TODO fix context_create_kernel
//		set_arg(th, th->attr.arg);

		ready_enqueue(th->priority, &(th->queue));
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
	run_task->status = TTS_DMT;
	leave_serialize();

	thread_switch();
}

void thread_end_and_destroy(void)
{
	thread_t *th;

	enter_serialize();
	th = run_task;

	tree_remove(&thread_tree, thread_id(th));
	list_remove(&(th->queue));

	if (th->attr.domain_id != KERNEL_DOMAIN_ID)
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
			result = E_OK;
			break;

		case TTS_WAI:
		case TTS_WAS:
			if (th->wait.type)
				list_remove(&(th->wait.waiting));
		case TTS_SUS:
			th->status = TTS_DMT;
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