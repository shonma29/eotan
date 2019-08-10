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

#define MIN_SID (1)
#define MIN_FID (1)
#define MIN_AUTO_FD (3)

#define getParent(type, p) ((intptr_t)p - offsetof(type, node))

static slab_t process_slab;
static tree_t process_tree;
static slab_t thread_slab;
static tree_t thread_tree;
static slab_t file_slab;
static slab_t descriptor_slab;
static slab_t process_group_slab;
static tree_t process_group_tree;
static slab_t session_slab;
static tree_t session_tree;

static void process_clear(mm_process_t *p);
static void thread_clear(mm_thread_t *th, mm_process_t *p);
static ER_ID create_thread(mm_process_t *p, FP entry, VP ustack_top);
static int process_find_new_pid(void);


mm_process_t *get_process(const ID pid)
{
	node_t *node = tree_get(&process_tree, pid);
	return (node ? (mm_process_t*)getParent(mm_process_t, node) : NULL);
}

mm_thread_t *get_thread(const ID tid)
{
	node_t *node = tree_get(&thread_tree, tid);
	return (node ? (mm_thread_t*)getParent(mm_thread_t, node) : NULL);
}

static inline mm_thread_t *getMyThread(const list_t *p)
{
	return (mm_thread_t*)((intptr_t)p - offsetof(mm_thread_t, brothers));
}
/*
static inline mm_process_t *getProcessFromChildren(const list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, children));
}
*/
static inline mm_process_t *getProcessFromBrothers(const list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, brothers));
}
/*
static inline mm_process_t *getProcessFromMembers(const list_t *p)
{
	return (mm_process_t*)((intptr_t)p - offsetof(mm_process_t, members));
}

static inline mm_process_group_t *getProcessGroupFromMembers(const list_t *p)
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

	//TODO take over fields when duplicate?
	list_initialize(&(p->threads));
	//TODO leave on exec
	tree_create(&(p->descriptors), NULL);
	tree_create(&(p->sessions), NULL);
	list_initialize(&(p->brothers));
	list_initialize(&(p->children));
	list_initialize(&(p->members));
	p->local = NULL;
	p->rdvno = 0;
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
	process_slab.max_block = slab_max_block(PROCESS_MAX, PAGE_SIZE,
			sizeof(mm_process_t));
	process_slab.palloc = kcall->palloc;
	process_slab.pfree = kcall->pfree;
	slab_create(&process_slab);

	tree_create(&process_tree, NULL);

	// initialize thread table
	thread_slab.unit_size = sizeof(mm_thread_t);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = slab_max_block(THREAD_MAX, PAGE_SIZE,
			sizeof(mm_thread_t));
	thread_slab.palloc = kcall->palloc;
	thread_slab.pfree = kcall->pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, NULL);

	// initialize file table
	file_slab.unit_size = sizeof(mm_file_t);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_file_t));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	slab_create(&file_slab);

	// initialize descriptor table
	descriptor_slab.unit_size = sizeof(mm_descriptor_t);
	descriptor_slab.block_size = PAGE_SIZE;
	descriptor_slab.min_block = 1;
	descriptor_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_descriptor_t));
	descriptor_slab.palloc = kcall->palloc;
	descriptor_slab.pfree = kcall->pfree;
	slab_create(&descriptor_slab);

	// initialize process group table
	process_group_slab.unit_size = sizeof(mm_process_group_t);
	process_group_slab.block_size = PAGE_SIZE;
	process_group_slab.min_block = 1;
	process_group_slab.max_block = slab_max_block(PROCESS_GROUP_MAX,
			PAGE_SIZE, sizeof(mm_process_group_t));
	process_group_slab.palloc = kcall->palloc;
	process_group_slab.pfree = kcall->pfree;
	slab_create(&process_group_slab);

	tree_create(&process_group_tree, NULL);

	// initialize session table
	session_slab.unit_size = sizeof(mm_session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = slab_max_block(SESSION_MAX, PAGE_SIZE,
			sizeof(mm_session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;
	slab_create(&session_slab);

	tree_create(&session_tree, NULL);
}

mm_process_t *process_duplicate(mm_process_t *src)
{
	do {
		pid_t pid = process_find_new_pid();
		if (pid == -1) {
			//TODO set adequate errno
//			reply->data[0] = ENOMEM;
			break;
		}

		mm_process_t *dest = get_process(pid);
		//TODO check duplicated process_id
		if (!dest) {
			dest = (mm_process_t*)slab_alloc(&process_slab);
			if (!dest) {
//				reply->data[0] = ENOMEM;
				break;
			}

			if (!tree_put(&process_tree,
					pid, (node_t*)dest)) {
				slab_free(&process_slab, dest);
//				reply->data[0] = EBUSY;
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
//			reply->data[0] = EFAULT;
			break;
		}

		dest->segments.heap = src->segments.heap;

		for (int fd = 0; fd < FILES_PER_PROCESS; fd++) {
			mm_descriptor_t *s = process_find_desc(src, fd);
			if (!s)
				continue;

			mm_descriptor_t *d = process_allocate_desc();
			if (!d)
				//TODO return error
				continue;

			if (process_set_desc(dest, fd, d))
				process_deallocate_desc(d);
			else {
				d->file = s->file;
				d->file->f_count++;
			}
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
		dest->session = src->session;
		dest->wd = src->wd;
		if (dest->wd)
			dest->wd->f_count++;

		strcpy(dest->name, src->name);

		if (map_user_pages(dest->directory, (void*)LOCAL_ADDR,
				pages(sizeof(*(dest->local))))) {
//			reply->data[0] = ENOMEM;
			break;
		}

		dest->local = getPageAddress(kern_p2v(dest->directory),
				(void*)LOCAL_ADDR);
		memset(dest->local, 0, sizeof(*(dest->local)));
		dest->local->pid = dest->node.key;
		dest->local->ppid = dest->ppid;
		dest->local->uid = dest->uid;
		dest->local->gid = dest->gid;
		dest->local->wd_len = src->local->wd_len;
		strcpy(dest->local->wd, src->local->wd);

		log_info("d %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s %x\n",
				dest->node.key,
				&dest->brothers, dest->brothers.next,
				&dest->members, dest->members.next,
				dest->ppid, dest->pgid, dest->uid, dest->gid,
				dest->name,
				(dest->wd) ? dest->wd->node.key : (-1));
		return dest;
	} while (false);

	return NULL;
}

int process_destroy(mm_process_t *proc, const int status)
{
	proc->exit_status = status;

	for (;;) {
		list_t *thread = list_pick(&(proc->threads));
		if (!thread)
			break;

		mm_thread_t *th = getMyThread(thread);
		kcall->thread_terminate(th->node.key);
		kcall->thread_destroy(th->node.key);
		tree_remove(&thread_tree, th->node.key);
	}

	mm_process_t *parent = get_process(proc->ppid);
	if (parent) {
		if (parent->rdvno) {
			log_info("mm: %d release parent %d\n",
					proc->node.key, proc->ppid);
			process_release_body(parent);
		} else {
			log_info("mm: %d parent not wait destroy\n",
					proc->node.key);
		}
	} else {
		//TODO what to do?
		log_info("mm: %d no parent destroy\n", proc->node.key);
	}

//TODO if current process is 'init'?
	parent = get_process(INIT_PID);
	if (parent) {
		bool found = false;
		for (;;) {
			list_t *child = list_pick(&(proc->children));
			if (!child)
				break;

			mm_process_t *p = getProcessFromBrothers(child);
			p->ppid = INIT_PID;
			list_append(&(parent->children), child);
			found = true;
		}

		if (found) {
			if (parent->rdvno)
				process_release_body(parent);
		}
	} else {
		//TODO what to do?
	}

	//TODO release resources
	return 0;
}

int process_release_body(mm_process_t *proc)
{
//TODO list_insert dead child on exit
	if (list_is_empty(&(proc->children))) {
		log_info("mm: %d no child release body\n", proc->node.key);
		return ECHILD;
	}

	for (list_t *child= list_next(&(proc->children));
			!list_is_edge(&(proc->children), child);
			child = list_next(child)) {
		mm_process_t *p = getProcessFromBrothers(child);
		if (!list_is_empty(&(p->threads)))
			continue;

		RDVNO rdvno = proc->rdvno;
		mm_reply_t reply = {
			p->node.key,
			{ p->exit_status & 0xff }
		};

		proc->rdvno = 0;
		list_remove(child);
		if (!tree_remove(&process_tree, p->node.key)) {
			//TODO what to do?
		}

		slab_free(&process_slab, p);

		ER result = kcall->port_reply(rdvno, &reply, sizeof(reply));
		if (result)
			log_err("mm: %d failed to release body(%d)\n",
					proc->node.key, result);

		log_info("mm: %d success release body\n", proc->node.key);
		return 0;
	}

	log_info("mm: %d not found release body\n", proc->node.key);
	return 0;
}

int mm_vmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_process_t *p = get_process((ID)args->arg1);
		if (!p) {
			reply->data[0] = ESRCH;
			break;
		}

		if (map_user_pages(p->directory,
				(VP)(args->arg2), pages((UW)(args->arg3)))) {
			reply->data[0] = ENOMEM;
			break;
		}

		unsigned int currentEnd = (unsigned int)(p->segments.heap.addr)
				+ p->segments.heap.len;
		unsigned int newEnd = (unsigned int)(args->arg2)
				+ (unsigned int)(args->arg3);
		if (currentEnd == (unsigned int)(args->arg2))
			p->segments.heap.len = newEnd
					- (unsigned int)(p->segments.heap.addr);

		if (args->arg2 == LOCAL_ADDR)
			p->local = getPageAddress(kern_p2v(p->directory),
					(void*)(args->arg2));

		reply->data[0] = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vunmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		unsigned int currentEnd;
		unsigned int newEnd;
		mm_process_t *p = get_process((ID)args->arg1);

		if (!p) {
			reply->data[0] = ESRCH;
			break;
		}

		if (unmap_user_pages(p->directory,
				(VP)(args->arg2), pages((UW)(args->arg3)))) {
			reply->data[0] = EACCES;
			break;
		}

		currentEnd = (unsigned int)(p->segments.heap.addr)
				+ p->segments.heap.len;
		newEnd = (unsigned int)(args->arg2)
				+ (unsigned int)(args->arg3);
		if (currentEnd == newEnd)
			p->segments.heap.len = (unsigned int)(args->arg2)
					- (unsigned int)(p->segments.heap.addr);

		reply->data[0] = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_sbrk(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *p = get_process(th->process_id);
		if (!p) {
			reply->data[0] = ESRCH;
			break;
		}

		mm_segment_t *s = &(p->segments.heap);
		uintptr_t end = (uintptr_t)(s->addr) + s->len;
		intptr_t diff = (intptr_t)(args->arg1);
		if (diff > 0) {
			diff = pageRoundUp(diff);
			if (s->max - s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (map_user_pages(p->directory,
					(VP)end, pages(diff))) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len += diff;
			end += diff;

		} else if (diff < 0) {
			diff = pageRoundUp(-diff);
			if (s->len < diff) {
				reply->data[0] = ENOMEM;
				break;
			}

			if (unmap_user_pages(p->directory,
					(VP)(end - diff), pages(diff))) {
				reply->data[0] = ENOMEM;
				break;
			}

			s->len -= diff;
			end -= diff;
		}

		reply->data[0] = EOK;
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

int thread_create(mm_process_t *process, FP entry, VP stack)
{
	do {
		ER_ID result;
		mm_thread_t *th;

		if (map_user_pages(process->directory,
				(VP)pageRoundDown(LOCAL_ADDR - USER_STACK_INITIAL_SIZE - PAGE_SIZE),
				pages(pageRoundUp(USER_STACK_INITIAL_SIZE)))) {
//			reply->data[0] = ENOMEM;
			break;
		}

		result = create_thread(process, entry, stack);
		if (result < 0) {
//			reply->data[0] = ECONNREFUSED;
			break;
		}

		//TODO check duplicated thread_id
		th = (mm_thread_t*)slab_alloc(&thread_slab);
		if (!th) {
			kcall->thread_destroy(result);
//			reply->data[0] = ENOMEM;
			break;
		}

		if (tree_put(&thread_tree, result, (node_t*)th))
			thread_clear(th, process);
		else {
			slab_free(&thread_slab, th);
			kcall->thread_destroy(result);
//			reply->data[0] = EBUSY;
			break;
		}

		//TODO only main thread
		if (process->local)
				process->local->thread_id = result;

		return result;
	} while (false);

//TODO unmap
	return -1;
}
//TODO delete thread
//TODO delete process

int mm_thread_find(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread((ID)args->arg1);
		if (!th) {
			reply->data[0] = ESRCH;
			break;
		}

		reply->data[0] = EOK;
		reply->result = th->process_id;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

mm_descriptor_t *process_create_file(void)
{
	mm_descriptor_t *d = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (d) {
		mm_file_t *f = process_allocate_file();
		if (!f) {
			slab_free(&descriptor_slab, d);
			return NULL;
		}

		d->file = f;
	}

	return d;
}

mm_file_t *process_allocate_file(void)
{
	return (mm_file_t*)slab_alloc(&file_slab);
}

void process_deallocate_file(mm_file_t *file)
{
	slab_free(&file_slab, file);
}

mm_descriptor_t *process_allocate_desc(void)
{
	mm_descriptor_t *d = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (d)
		//TODO is needed?
		d->file = NULL;

	return d;
}

void process_deallocate_desc(mm_descriptor_t *desc)
{
	slab_free(&descriptor_slab, desc);
}

int process_set_desc(mm_process_t *process, const int fd, mm_descriptor_t *desc)
{
	return (tree_put(&(process->descriptors), fd, &(desc->node)) ? 0 : 1);
}

int process_destroy_desc(mm_process_t *process, const int fd)
{
	mm_descriptor_t *desc =
			(mm_descriptor_t*)tree_remove(&(process->descriptors),
					fd);
	if (!desc)
		return EBADF;

	if (desc->file)
		//TODO close here?
		process_deallocate_file(desc->file);

	process_deallocate_desc(desc);
	return 0;
}

mm_descriptor_t *process_find_desc(const mm_process_t *process, const int fd)
{
	return (mm_descriptor_t*)tree_get(&(process->descriptors), fd);
}

static int process_find_new_pid(void)
{
	for (int id = INIT_PID; id < PROCESS_MAX; id++)
		if (!tree_get(&process_tree, id))
			return id;

	return -1;
}

int process_find_new_fd(const mm_process_t *process)
{
	for (int id = MIN_AUTO_FD; id < FILES_PER_PROCESS; id++)
		if (!process_find_desc(process, id))
			return id;

	return -1;
}

int create_init(const pid_t pid)
{
	//TODO check duplicated process_id
	//TODO key must not be 0

	mm_process_t *p = (mm_process_t*)slab_alloc(&process_slab);
	if (!p)
		return ENOMEM;

	if (!tree_put(&process_tree, pid, (node_t*)p)) {
		slab_free(&process_slab, p);
		return EBUSY;
	}

	//TODO adhoc
	p->directory = NULL;
	process_clear(p);

	//TODO check NULL
	p->directory = copy_kernel_page_table();

	unsigned int start = pageRoundDown(0);
	unsigned int end = pageRoundUp((unsigned int)0 + (unsigned int)0);
	if (map_user_pages(p->directory, (VP)start, pages(end - start)))
		return ENOMEM;

	p->segments.heap.addr = (void*)end;
	p->segments.heap.len = 0;
	p->segments.heap.max = pageRoundUp(0) - end;
	p->segments.heap.attr = type_heap;

	p->ppid = INIT_PPID;
	p->pgid = INIT_PID;
	p->uid = INIT_UID;
	p->gid = INIT_GID;
	p->session = NULL;
	p->wd = NULL;

	mm_process_group_t *pg = slab_alloc(&process_group_slab);
	if (pg) {
		list_initialize(&(pg->members));
		if (!tree_put(&process_group_tree, INIT_PID, (node_t*)pg)) {
			//TODO what to do?
			slab_free(&process_group_slab, pg);
		}
	}

	if (map_user_pages(p->directory, (void*)LOCAL_ADDR,
			pages(sizeof(*(p->local))))) {
		return ENOMEM;
	}

	p->local = getPageAddress(kern_p2v(p->directory), (void*)LOCAL_ADDR);
	memset(p->local, 0, sizeof(*(p->local)));
	p->local->pid = p->node.key;
	p->local->ppid = p->ppid;
	p->local->uid = p->uid;
	p->local->gid = p->gid;
	p->local->wd_len = 1;
	strcpy(p->local->wd, "/");

	mm_descriptor_t *d = process_create_file();
	if (d) {
		mm_file_t *f = d->file;
		f->server_id = PORT_CONSOLE;
		//TODO open
		f->node.key = 0;
		f->f_flag = O_RDONLY;
		f->f_count = 1;
		f->f_offset = 0;
		if (process_set_desc(p, STDIN_FILENO, d)) {
			//TODO what to do?
			process_deallocate_file(f);
			process_deallocate_desc(d);
		}
	}

	d = process_create_file();
	if (d) {
		mm_file_t *f = d->file;
		f->server_id = PORT_CONSOLE;
		//TODO open
		f->node.key = 0;
		f->f_flag = O_WRONLY;
		f->f_count = 1;
		f->f_offset = 0;
		if (process_set_desc(p, STDOUT_FILENO, d)) {
			//TODO what to do?
			process_deallocate_file(f);
			process_deallocate_desc(d);
		}

		d = process_allocate_desc();
		if (d) {
			if (process_set_desc(p, STDERR_FILENO, d))
				//TODO what to do?
				process_deallocate_desc(d);
			else {
				d->file = f;
				f->f_count++;
			}
		}
	}
#if 0
	log_info("c %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s\n",
			p->node.key,
			&p->brothers, p->brothers.next,
			&p->members, p->members.next,
			p->ppid, p->pgid, p->uid, p->gid,
			p->name);
#endif
	return 0;
}

int process_replace(mm_process_t *process,
		void *address, const size_t size,
		void *entry, const void *args, const size_t stack_size,
		int *thread_id)
{
	if (unmap_user_pages(process->directory,
			//TODO this address is adhoc. fix region_unmap
			(VP)0x1000,
			pages((unsigned int)(process->segments.heap.addr)
					+  process->segments.heap.len))) {
		return EFAULT;
	}

	process->segments.heap.addr = NULL;
	process->segments.heap.len = 0;
	process->segments.heap.max = 0;
	process->segments.heap.attr = attr_nil;

	unsigned int start = pageRoundDown((unsigned int)address);
	unsigned int end = pageRoundUp((unsigned int)(address)
			+ (unsigned int)(size));

	if (map_user_pages(process->directory,
			(VP)start, pages(end - start))) {
		return ENOMEM;
	}

	process->segments.heap.addr = (void*)end;
	process->segments.heap.len = 0;
	process->segments.heap.max = pageRoundUp(USER_HEAP_MAX_ADDR) - end;
	process->segments.heap.attr = type_heap;

	if (stack_size > USER_STACK_INITIAL_SIZE) {
		return E2BIG;
	}

	if (process->node.key == INIT_PID) {
		if (map_user_pages(process->directory,
				(void*)pageRoundDown(LOCAL_ADDR - USER_STACK_INITIAL_SIZE - PAGE_SIZE),
				pages(pageRoundUp(USER_STACK_INITIAL_SIZE)))) {
			return ENOMEM;
		}
	}

	while (!list_is_empty(&(process->threads))) {
		list_t *n = list_next(&(process->threads));
		mm_thread_t *th = getMyThread(n);

		list_remove(n);
		kcall->thread_terminate(th->node.key);
		kcall->thread_destroy(th->node.key);
		tree_remove(&thread_tree, th->node.key);
		//TODO destroy mm_thread_t
	}

	unsigned int stack_top = pageRoundDown(LOCAL_ADDR - PAGE_SIZE)
			- stack_size;
	if (move_stack(process->directory, (void*)stack_top,
			(void*)args, stack_size)) {
		return EFAULT;
	}

	ER_ID result = create_thread(process, (FP)entry,
			(VP)(stack_top - sizeof(int)));
	if (result < 0) {
		return ECONNREFUSED;
	}

	if (process->local)
			process->local->thread_id = result;

	//TODO check duplicated thread_id
	mm_thread_t *th = (mm_thread_t*)slab_alloc(&thread_slab);
	if (!th) {
		kcall->thread_destroy(result);
		return ENOMEM;
	}

	if (tree_put(&thread_tree, result, (node_t*)th))
		thread_clear(th, process);
	else {
		slab_free(&thread_slab, th);
		kcall->thread_destroy(result);
		return EBUSY;
	}

	*thread_id = th->node.key;

	return 0;
}

mm_session_t *session_create(void)
{
	int id;
	for (id = MIN_SID; id < SESSION_MAX; id++)
		if (!tree_get(&session_tree, id))
			break;

	if (id == SESSION_MAX)
		return NULL;

	mm_session_t *session = (mm_session_t*)slab_alloc(&session_slab);
	if (session) {
		if (!tree_put(&session_tree, id, &(session->node))) {
			//TODO what to do?
		}

		tree_create(&(session->files), NULL);
		session->refer_count = 1;
	}

	return session;
}

int session_destroy(mm_session_t *session)
{
	session->refer_count--;
	if (!(session->refer_count)) {
		//TODO warn and release opened files
		//TODO clunk root fid if opened

		if (!tree_remove(&session_tree, session->node.key)) {
			//TODO what to do?
		}

		slab_free(&session_slab, session);
	}

	return 0;
}

int session_find_new_fid(mm_session_t *session)
{
//log_info("mm: ffile %d\n", session->node.key);
	for (int fid = MIN_FID; fid < FILES_PER_SESSION; fid++)
		if (!tree_get(&(session->files), fid))
			return fid;

	return -1;
}

int session_add_file(mm_session_t *session, const int fid, mm_file_t *file)
{
//log_info("mm: afile sid=%d fid=%d\n", session->node.key, fid);
	return (tree_put(&(session->files), fid, &(file->node)) ? 0 : 1);
}

int session_remove_file(mm_session_t *session, const int fid)
{
//log_info("mm: rfile sid=%d fid=%d\n", session->node.key, fid);
	mm_file_t *file = (mm_file_t*)tree_remove(&(session->files), fid);
	if (!file)
		return EBADF;

	return 0;
}
