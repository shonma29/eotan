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
#include <errno.h>
#include <mm.h>
#include <stdint.h>
#include <boot/init.h>
#include <core/options.h>
#include <mm/segment.h>
#include <nerve/kcall.h>
#include <set/list.h>
#include <set/tree.h>
#include "interface.h"
#include "process.h"
#include "../../kernel/mpu/mpufunc.h"

#define getParent(type, p) ((ptr_t)p - offsetof(type, node))

static kcall_t *kcall = (kcall_t*)KCALL_ADDR;
static slab_t process_slab;
static tree_t process_tree;
static slab_t thread_slab;
static tree_t thread_tree;

static mm_process_t *get_process(const ID pid);
static mm_thread_t *get_thread(const ID tid);
static mm_thread_t *getMyThread(list_t *brothers);
static void process_clear(mm_process_t *p);
static void thread_clear(mm_thread_t *th, mm_process_t *p);

static mm_process_t *get_process(const ID pid)
{
	node_t *node = tree_get(&process_tree, pid);

	return node? (mm_process_t*)getParent(mm_process_t, node):NULL;
}

static mm_thread_t *get_thread(const ID tid)
{
	node_t *node = tree_get(&thread_tree, tid);

	return node? (mm_thread_t*)getParent(mm_thread_t, node):NULL;
}

static mm_thread_t *getMyThread(list_t *p)
{
	return (mm_thread_t*)((ptr_t)p - offsetof(mm_thread_t, brothers));
}

static void process_clear(mm_process_t *p)
{
	p->segments.heap.attr = attr_nil;
	p->directory = NULL;
	list_initialize(&(p->threads));
}

static void thread_clear(mm_thread_t *th, mm_process_t *p)
{
	th->process_id = p->node.key;
	th->stack.attr = attr_nil;
	list_initialize(&(th->brothers));
	list_append(&(p->threads), &(th->brothers));
}

void process_initialize(void)
{
	/* initialize process table */
	process_slab.unit_size = sizeof(mm_process_t);
	process_slab.block_size = PAGE_SIZE;
	process_slab.min_block = 1;
	process_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(mm_process_t));
	process_slab.palloc = kcall->palloc;
	process_slab.pfree = kcall->pfree;
	slab_create(&process_slab);

	tree_create(&process_tree, &process_slab, NULL);

	/* initialize thread table */
	thread_slab.unit_size = sizeof(mm_thread_t);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(mm_thread_t));
	thread_slab.palloc = kcall->palloc;
	thread_slab.pfree = kcall->pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, &thread_slab, NULL);
}

int mm_process_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		size_t start;
		size_t end;
		list_t *list;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		list = list_head(&(p->threads));
		if (!list) {
			reply->error_no = ESRCH;
			break;
		}

		start = pageRoundDown(args->arg2);
		end = pageRoundUp((size_t)(args->arg2) + (size_t)(args->arg3));

		if (kcall->region_map((ID)(getMyThread(list)->node.key),
				(VP)start, end - start, true)) {
			reply->error_no = ESVC;
			break;
		}

		p->segments.heap.addr = (void*)end;
		p->segments.heap.len = 0;
		p->segments.heap.max = pageRoundUp(args->arg4) - end;
		p->segments.heap.attr = type_heap;

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_destroy(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		list_t *list;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		list = list_head(&(p->threads));
		if (!list) {
			reply->error_no = ESRCH;
			break;
		}

		if (kcall->region_unmap(getMyThread(list)->node.key,
				//TODO this address is adhoc. fix region_unmap
				(VP)0x1000,
				(size_t)(p->segments.heap.addr)
						+  p->segments.heap.len)) {
			reply->error_no = EFAULT;
			break;
		}

		p->segments.heap.addr = NULL;
		p->segments.heap.len = 0;
		p->segments.heap.max = 0;
		p->segments.heap.attr = attr_nil;

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_duplicate(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_process_t *dest;
		mm_process_t *src = get_process((ID)args->arg1);

		if (!src) {
			reply->error_no = ESRCH;
			break;
		}

		dest = get_process((ID)args->arg2);
		if (!dest) {
			reply->error_no = ESRCH;
			break;
		}

		if (copy_user_pages(dest->directory, src->directory,
				pageRoundUp((UW)(src->segments.heap.addr)
						+ src->segments.heap.len)
						>> BITS_OFFSET)) {
			reply->error_no = EFAULT;
			break;
		}

		dest->segments.heap = src->segments.heap;

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_copy_stack(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *dest;
		mm_thread_t *src = get_thread((ID)args->arg1);

		if (!src) {
			reply->error_no = ESRCH;
			break;
		}

		dest = get_thread((ID)args->arg3);
		if (!dest) {
			reply->error_no = ESRCH;
			break;
		}

		if (kcall->mpu_copy_stack((ID)(args->arg1), (W)(args->arg2),
				(ID)(args->arg3))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_set_context(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread((ID)args->arg1 & INIT_THREAD_ID_MASK);

		if (!th) {
			reply->error_no = ESRCH;
			break;
		}

		if (kcall->mpu_set_context((ID)(args->arg1), (W)(args->arg2),
				(B*)(args->arg3), (W)(args->arg4))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		size_t currentEnd;
		size_t newEnd;
		list_t *list;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		list = list_head(&(p->threads));
		if (!list) {
			reply->error_no = ESRCH;
			break;
		}

		if (kcall->region_map((ID)(getMyThread(list)->node.key),
				(VP)(args->arg2), (UW)(args->arg3),
				(W)(args->arg4))) {
			reply->error_no = ESVC;
			break;
		}

		currentEnd = (size_t)(p->segments.heap.addr)
				+ p->segments.heap.len;
		newEnd = (size_t)(args->arg2) + (size_t)(args->arg3);
		if (currentEnd == (size_t)(args->arg2))
			p->segments.heap.len = newEnd
					- (size_t)(p->segments.heap.addr);

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vunmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		size_t currentEnd;
		size_t newEnd;
		list_t *list;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		list = list_head(&(p->threads));
		if (!list) {
			reply->error_no = ESRCH;
			break;
		}

		if (kcall->region_unmap((ID)(getMyThread(list)->node.key),
				(VP)(args->arg2), (UW)(args->arg3))) {
			reply->error_no = ESVC;
			break;
		}

		currentEnd = (size_t)(p->segments.heap.addr)
				+ p->segments.heap.len;
		newEnd = (size_t)(args->arg2) + (size_t)(args->arg3);
		if (currentEnd == newEnd)
			p->segments.heap.len = (size_t)(args->arg2)
					- (size_t)(p->segments.heap.addr);

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_sbrk(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_segment_t *s;
		intptr_t diff;
		uintptr_t end;
		list_t *list;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		list = list_head(&(p->threads));
		if (!list) {
			reply->error_no = ESRCH;
			break;
		}

		s = &(p->segments.heap);
		end = (uintptr_t)(s->addr) + s->len;
		diff = (intptr_t)(args->arg2);
		if (diff > 0) {
			diff = pageRoundUp(diff);
			if (s->max - s->len < diff) {
				reply->error_no = ENOMEM;
				break;
			}

			if (kcall->region_map((ID)(getMyThread(list)->node.key),
					(VP)end, diff, true)) {
				reply->error_no = ENOMEM;
				break;
			}

			s->len += diff;
			end += diff;

		} else if (diff < 0) {
			diff = pageRoundUp(-diff);
			if (s->len < diff) {
				reply->error_no = ENOMEM;
				break;
			}

			if (kcall->region_unmap((ID)(getMyThread(list)->node.key),
					(VP)(end - diff), diff)) {
				reply->error_no = ENOMEM;
				break;
			}

			s->len -= diff;
			end -= diff;
		}

		reply->error_no = EOK;
		reply->result = (int)end;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_thread_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		T_CTSK pk_ctsk = {
			TA_HLNG,
			NULL,
			NULL,
			pri_user_foreground,
			KTHREAD_STACK_SIZE,
			NULL,
			0
		};
		ER_ID result;
		mm_thread_t *th;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			p = (mm_process_t*)tree_put(&process_tree,
					(ID)args->arg1);

			if (!p) {
				reply->error_no = ENOMEM;
				break;
			}

			process_clear(p);
		}

		pk_ctsk.task = (FP)(args->arg2);
		//TODO check NULL
		p->directory = copy_kernel_page_table();
		pk_ctsk.page_table = p->directory;

		result = kcall->thread_create_auto(&pk_ctsk);
		if (result < 0) {
			reply->error_no = ESVC;
			break;
		}

		reply->error_no = EOK;
		reply->result = result;

		//TODO check duplicated thread_id
		th = (mm_thread_t*)tree_put(&thread_tree, result);
		if (!th) {
			kcall->thread_destroy(result);
			reply->error_no = ENOMEM;
			break;
		}

		thread_clear(th, p);
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}
//TODO delete thread
//TODO delete process
