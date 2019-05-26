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
#include <errno.h>
#include <fcntl.h>
#include <mm.h>
#include <services.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <boot/init.h>
#include <core/options.h>
#include <mm/config.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include "interface.h"
#include "process.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"

#define MIN_AUTO_FD (3)

#define getParent(type, p) ((intptr_t)p - offsetof(type, node))

static slab_t process_slab;
static tree_t process_tree;
static slab_t thread_slab;
static tree_t thread_tree;
static slab_t file_slab;
static slab_t process_group_slab;
static tree_t process_group_tree;
static slab_t session_slab;

static mm_thread_t *getMyThread(list_t *brothers);
static void process_clear(mm_process_t *p);
static void thread_clear(mm_thread_t *th, mm_process_t *p);
static ER_ID create_thread(mm_process_t *p, FP entry, VP ustack_top);


mm_process_t *get_process(const ID pid)
{
	node_t *node = tree_get(&process_tree, pid);

	return node? (mm_process_t*)getParent(mm_process_t, node):NULL;
}

mm_thread_t *get_thread(const ID tid)
{
	node_t *node = tree_get(&thread_tree, tid);

	return node? (mm_thread_t*)getParent(mm_thread_t, node):NULL;
}

static mm_thread_t *getMyThread(list_t *p)
{
	return (mm_thread_t*)((intptr_t)p - offsetof(mm_thread_t, brothers));
}
/*
static mm_process_t *getProcessFromChildren(list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, children));
}

static mm_process_t *getProcessFromBrothers(list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, brothers));
}

static mm_process_t *getProcessFromMembers(list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, members));
}

static mm_process_group_t *getProcessGroupFromMembers(list_t *p)
{
	return (mm_process_group_t*)((intptr_t)p
			- offsetof(mm_process_group_t, members));
}
*/

static void process_clear(mm_process_t *p)
{
	p->segments.heap.attr = attr_nil;

	if (p->directory) {
		kcall->pfree(p->directory);
		p->directory = NULL;
	}

	list_initialize(&(p->threads));
	//TODO leave on exec
	tree_create(&(p->files), NULL);
	tree_create(&(p->sessions), NULL);
	list_initialize(&(p->brothers));
	list_initialize(&(p->children));
	list_initialize(&(p->members));
	p->local = NULL;
	p->wait.rdvno = 0;
	p->name[0] = '\0';
}

static void thread_clear(mm_thread_t *th, mm_process_t *p)
{
	th->process_id = p->node.key;
	th->stack.attr = attr_nil;
	list_append(&(p->threads), &(th->brothers));
}

void process_initialize(void)
{
	// initialize process table
	process_slab.unit_size = sizeof(mm_process_t);
	process_slab.block_size = PAGE_SIZE;
	process_slab.min_block = 1;
	process_slab.max_block = tree_max_block(PROCESS_MAX, PAGE_SIZE,
			sizeof(mm_process_t));
	process_slab.palloc = kcall->palloc;
	process_slab.pfree = kcall->pfree;
	slab_create(&process_slab);

	tree_create(&process_tree, NULL);

	// initialize thread table
	thread_slab.unit_size = sizeof(mm_thread_t);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = tree_max_block(THREAD_MAX, PAGE_SIZE,
			sizeof(mm_thread_t));
	thread_slab.palloc = kcall->palloc;
	thread_slab.pfree = kcall->pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, NULL);

	// initialize file table
	file_slab.unit_size = sizeof(mm_file_t);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = tree_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_file_t));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	slab_create(&file_slab);

	// initialize process group table
	process_group_slab.unit_size = sizeof(mm_process_group_t);
	process_group_slab.block_size = PAGE_SIZE;
	process_group_slab.min_block = 1;
	process_group_slab.max_block = tree_max_block(PROCESS_GROUP_MAX,
			PAGE_SIZE, sizeof(mm_process_group_t));
	process_group_slab.palloc = kcall->palloc;
	process_group_slab.pfree = kcall->pfree;
	slab_create(&process_group_slab);

	tree_create(&process_group_tree, NULL);

	// initialize session table
	session_slab.unit_size = sizeof(mm_session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = tree_max_block(SESSION_MAX, PAGE_SIZE,
			sizeof(mm_session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;
	slab_create(&session_slab);
}

int mm_process_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		size_t start;
		size_t end;
		mm_process_t *p = get_process((ID)args->arg1);

		//TODO check duplicated process_id
		if (!p) {
			p = (mm_process_t*)slab_alloc(&process_slab);
			if (!p) {
				reply->error_no = ENOMEM;
				break;
			}

			if (!tree_put(&process_tree,
					(ID)args->arg1, (node_t*)p)) {
				slab_free(&process_slab, p);
				reply->error_no = EBUSY;
				break;
			}

			//TODO adhoc
			p->directory = NULL;
			process_clear(p);

			//TODO check NULL
			p->directory = copy_kernel_page_table();
		}

		start = pageRoundDown(args->arg2);
		end = pageRoundUp((size_t)(args->arg2) + (size_t)(args->arg3));

		if (map_user_pages(p->directory,
				(VP)start, pages(end - start))) {
			reply->error_no = ENOMEM;
			break;
		}

		p->segments.heap.addr = (void*)end;
		p->segments.heap.len = 0;
		p->segments.heap.max = pageRoundUp(args->arg4) - end;
		p->segments.heap.attr = type_heap;

		if (p->node.key == INIT_PID) {
			p->ppid = INIT_PPID;
			p->pgid = INIT_PID;
			p->uid = INIT_UID;
			p->gid = INIT_GID;

			mm_process_group_t *pg =
					slab_alloc(&process_group_slab);
			if (pg) {
				list_initialize(&(pg->members));
				if (!tree_put(&process_group_tree, INIT_PID,
						(node_t*)pg)) {
					//TODO what to do?
					slab_free(&process_group_slab, pg);
				}
			}

			mm_file_t *f = process_allocate_desc();
			if (f) {
				f->server_id = PORT_CONSOLE;
				f->fid = 0;
				f->f_flag = O_RDONLY;
				f->f_count = 1;
				f->f_offset = 0;
				if (process_set_desc(p, STDIN_FILENO, f))
					process_deallocate_desc(f);
			}

			f = process_allocate_desc();
			if (f) {
				f->server_id = PORT_CONSOLE;
				f->fid = 0;
				f->f_flag = O_WRONLY;
				f->f_count = 1;
				f->f_offset = 0;
				if (process_set_desc(p, STDOUT_FILENO, f))
					process_deallocate_desc(f);
			}

			f = process_allocate_desc();
			if (f) {
				f->server_id = PORT_CONSOLE;
				f->fid = 0;
				f->f_flag = O_WRONLY;
				f->f_count = 1;
				f->f_offset = 0;
				if (process_set_desc(p, STDERR_FILENO, f))
					process_deallocate_desc(f);
			}
		}

log_notice("c %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s\n",
		p->node.key,
		&p->brothers, p->brothers.next,
		&p->members, p->members.next,
		p->ppid, p->pgid, p->uid, p->gid,
		p->name);

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
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		//TODO close on exit. not exec
		//TODO optimize
//		for (int fd = 0; fd < FILES_PER_PROCESS; fd++)
//			if (process_destroy_desc(p, fd)) {
//				//TODO what to do?
//			}

		if (unmap_user_pages(p->directory,
				//TODO this address is adhoc. fix region_unmap
				(VP)0x1000,
				pages((size_t)(p->segments.heap.addr)
						+  p->segments.heap.len))) {
			reply->error_no = EFAULT;
			break;
		}

		p->segments.heap.addr = NULL;
		p->segments.heap.len = 0;
		p->segments.heap.max = 0;
		p->segments.heap.attr = attr_nil;

//		kcall->pfree(p->directory);

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
		//TODO check duplicated process_id
		if (!dest) {
			dest = (mm_process_t*)slab_alloc(&process_slab);
			if (!dest) {
				reply->error_no = ENOMEM;
				break;
			}

			if (!tree_put(&process_tree,
					(ID)args->arg2, (node_t*)dest)) {
				slab_free(&process_slab, dest);
				reply->error_no = EBUSY;
				break;
			}

			process_clear(dest);

			//TODO check NULL
			dest->directory = copy_kernel_page_table();
		}

		if (copy_user_pages(dest->directory, src->directory,
//TODO copy only current stack
				pageRoundUp(LOCAL_ADDR - PAGE_SIZE)
//				pageRoundUp((UW)(src->segments.heap.addr)
//						+ src->segments.heap.len)
						>> BITS_OFFSET)) {
			reply->error_no = EFAULT;
			break;
		}

		dest->segments.heap = src->segments.heap;

		for (int fd = 0; fd < FILES_PER_PROCESS; fd++) {
			mm_file_t *s = process_find_desc(src, fd);
			if (!s)
				continue;

			mm_file_t *d = process_allocate_desc();
			if (!d)
				continue;

			d->server_id = s->server_id;
			d->fid = s->fid;
			d->f_flag = s->f_flag;
			d->f_count = s->f_count;
			d->f_offset = s->f_offset;
//TODO attach
//TODO walk
//TODO open
			if (process_set_desc(dest, fd, d))
				process_deallocate_desc(d);
		}

		list_append(&(src->children), &(dest->brothers));
		node_t *leader = tree_get(&process_group_tree, INIT_PID);
		if (leader) {
			list_append(&((mm_process_group_t*)leader)->members,
					&(dest->members));
		}
		dest->ppid = src->node.key;
		dest->pgid = src->pgid;
		dest->uid = src->uid;
		dest->gid = src->gid;
		//TODO attach -> walk -> set cwd
		//TODO set local
		strcpy(dest->name, src->name);

log_notice("d %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s\n",
		dest->node.key,
		&dest->brothers, dest->brothers.next,
		&dest->members, dest->members.next,
		dest->ppid, dest->pgid, dest->uid, dest->gid,
		dest->name);
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
		ER_ID result;
		size_t stack_size;
		size_t stack_top;
		mm_thread_t *th;
		mm_process_t *proc = get_process((ID)args->arg1);

		if (!proc) {
			reply->error_no = ESRCH;
			break;
		}

		stack_size = (size_t)(args->arg4);
		if (stack_size > USER_STACK_INITIAL_SIZE) {
			reply->error_no = E2BIG;
			break;
		}

		if (args->arg1 == INIT_PID) {
			if (map_user_pages(proc->directory,
					(VP)pageRoundDown(LOCAL_ADDR - USER_STACK_INITIAL_SIZE - PAGE_SIZE),
					pages(pageRoundUp(USER_STACK_INITIAL_SIZE)))) {
				reply->error_no = ENOMEM;
				break;
			}
		}

		while (!list_is_empty(&(proc->threads))) {
			list_t *n = list_next(&(proc->threads));
			mm_thread_t *th = getMyThread(n);

			list_remove(n);
			kcall->thread_terminate(th->node.key);
			kcall->thread_destroy(th->node.key);
			tree_remove(&thread_tree, th->node.key);
		}

		stack_top = pageRoundDown(LOCAL_ADDR - PAGE_SIZE) - stack_size;

		if (move_stack(proc->directory, (void*)stack_top,
				(void*)(args->arg3), stack_size)) {
			reply->error_no = EFAULT;
			break;
		}

		result = create_thread(proc, (FP)(args->arg2),
				(VP)(stack_top - sizeof(int)));
		if (result < 0) {
			reply->error_no = ECONNREFUSED;
			break;
		}

		//TODO check duplicated thread_id
		th = (mm_thread_t*)slab_alloc(&thread_slab);
		if (!th) {
			kcall->thread_destroy(result);
			reply->error_no = ENOMEM;
			break;
		}

		if (tree_put(&thread_tree, result, (node_t*)th))
			thread_clear(th, proc);
		else {
			slab_free(&thread_slab, th);
			kcall->thread_destroy(result);
			reply->error_no = EBUSY;
			break;
		}

		reply->error_no = EOK;
		reply->result = result;
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
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		if (map_user_pages(p->directory,
				(VP)(args->arg2), pages((UW)(args->arg3)))) {
			reply->error_no = ENOMEM;
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
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		if (unmap_user_pages(p->directory,
				(VP)(args->arg2), pages((UW)(args->arg3)))) {
			reply->error_no = EACCES;
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
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
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

			if (map_user_pages(p->directory,
					(VP)end, pages(diff))) {
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

			if (unmap_user_pages(p->directory,
					(VP)(end - diff), pages(diff))) {
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

static ER_ID create_thread(mm_process_t *p, FP entry, VP ustack_top)
{
	T_CTSK pk_ctsk = {
		TA_HLNG,
		(VP_INT)NULL,
		entry,
		pri_user_foreground,
		KTHREAD_STACK_SIZE,
		NULL,
		p->directory,
		ustack_top
	};

	p->segments.stack.addr = (void*)pageRoundDown(LOCAL_ADDR
			- USER_STACK_MAX_SIZE);
	p->segments.stack.len = pageRoundUp(USER_STACK_INITIAL_SIZE);
	p->segments.stack.max = pageRoundUp(USER_STACK_MAX_SIZE - PAGE_SIZE);
	p->segments.stack.attr = type_stack;

	return kcall->thread_create_auto(&pk_ctsk);
}

int mm_thread_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		ER_ID result;
		mm_thread_t *th;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->error_no = ESRCH;
			break;
		}

		if (map_user_pages(p->directory,
				(VP)pageRoundDown(LOCAL_ADDR - USER_STACK_INITIAL_SIZE - PAGE_SIZE),
				pages(pageRoundUp(USER_STACK_INITIAL_SIZE)))) {
			reply->error_no = ENOMEM;
			break;
		}

		result = create_thread(p, (FP)(args->arg2), (VP)(args->arg3));
		if (result < 0) {
			reply->error_no = ECONNREFUSED;
			break;
		}

		//TODO check duplicated thread_id
		th = (mm_thread_t*)slab_alloc(&thread_slab);
		if (!th) {
			kcall->thread_destroy(result);
			reply->error_no = ENOMEM;
			break;
		}

		if (tree_put(&thread_tree, result, (node_t*)th))
			thread_clear(th, p);
		else {
			slab_free(&thread_slab, th);
			kcall->thread_destroy(result);
			reply->error_no = EBUSY;
			break;
		}

		reply->error_no = EOK;
		reply->result = result;
		return reply_success;
	} while (FALSE);

//TODO unmap
	reply->result = -1;
	return reply_failure;
}
//TODO delete thread
//TODO delete process

int mm_thread_find(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread((ID)args->arg1);
		if (!th) {
			reply->error_no = ESRCH;
			break;
		}

		reply->error_no = EOK;
		reply->result = th->process_id;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

mm_file_t *process_allocate_desc(void)
{
	return (mm_file_t*)slab_alloc(&file_slab);
}

void process_deallocate_desc(mm_file_t *desc)
{
	slab_free(&file_slab, desc);
}

int process_set_desc(mm_process_t *process, const int fd, mm_file_t *desc)
{
	return tree_put(&(process->files), fd, &(desc->node))? 0:1;
}

int process_destroy_desc(mm_process_t *process, const int fd)
{
	mm_file_t *file = (mm_file_t*)tree_remove(&(process->files), fd);
	if (!file)
		return EBADF;

	slab_free(&file_slab, file);
	return 0;
}

mm_file_t *process_find_desc(const mm_process_t *process, const int fd)
{
	return (mm_file_t*)tree_get(&(process->files), fd);
}
