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
#include <init.h>
#include <services.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mm/config.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "api.h"
#include "process.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"

#define THREAD_LOOKUP_SIZE (16)

#define getParent(type, p) ((uintptr_t) p - offsetof(type, node))

static slab_t process_slab;
static tree_t process_tree;
static slab_t thread_slab;
static tree_t thread_tree;
static node_t *thread_lookup[THREAD_LOOKUP_SIZE];
static slab_t process_group_slab;
static tree_t process_group_tree;

static inline mm_thread_t *getMyThread(const list_t *p)
{
	return ((mm_thread_t *) ((uintptr_t) p
			- offsetof(mm_thread_t, brothers)));
}
/*
static inline mm_process_t *getProcessFromChildren(const list_t *p)
{
	return ((mm_process_t *) ((uintptr_t) p
			- offsetof(mm_process_t, children)));
}
*/
static inline mm_process_t *getProcessFromBrothers(const list_t *p)
{
	return ((mm_process_t *) ((uintptr_t) p
			- offsetof(mm_process_t, brothers)));
}
/*
static inline mm_process_t *getProcessFromMembers(const list_t *p)
{
	return ((mm_process_t *) ((uintptr_t) p
			- offsetof(mm_process_t, members)));
}

static inline mm_process_group_t *getProcessGroupFromMembers(const list_t *p)
{
	return ((mm_process_group_t *) ((uintptr_t) p
			- offsetof(mm_process_group_t, members));
}
*/

static node_t **thread_lookup_selector(const tree_t *, const int);
static int process_find_new_pid(void);
static int process_create(mm_process_t **, const int);
static void set_local(mm_process_t *, const char *, const size_t);
static int map_user_stack(mm_process_t *);
static int release_memory(mm_process_t *);
static int create_thread(int *, mm_process_t *, FP, VP);
static void destroy_threads(mm_process_t *);
static int create_init_thread(mm_process_t *, const FP);


static node_t **thread_lookup_selector(const tree_t *tree, const int key)
{
	return &(((node_t **) tree->root)[key & (THREAD_LOOKUP_SIZE - 1)]);
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

	tree_create(&process_tree, NULL, NULL);

	// initialize thread table
	thread_slab.unit_size = sizeof(mm_thread_t);
	thread_slab.block_size = PAGE_SIZE;
	thread_slab.min_block = 1;
	thread_slab.max_block = slab_max_block(THREAD_MAX, PAGE_SIZE,
			sizeof(mm_thread_t));
	thread_slab.palloc = kcall->palloc;
	thread_slab.pfree = kcall->pfree;
	slab_create(&thread_slab);

	tree_create(&thread_tree, NULL, thread_lookup_selector);
	tree_initialize_lookup(&thread_tree, thread_lookup, THREAD_LOOKUP_SIZE);

	// initialize process group table
	process_group_slab.unit_size = sizeof(mm_process_group_t);
	process_group_slab.block_size = PAGE_SIZE;
	process_group_slab.min_block = 1;
	process_group_slab.max_block = slab_max_block(PROCESS_GROUP_MAX,
			PAGE_SIZE, sizeof(mm_process_group_t));
	process_group_slab.palloc = kcall->palloc;
	process_group_slab.pfree = kcall->pfree;
	slab_create(&process_group_slab);

	tree_create(&process_group_tree, NULL, NULL);
}

mm_process_t *process_find(const ID pid)
{
	node_t *node = tree_get(&process_tree, pid);
	return (node ? (mm_process_t *) getParent(mm_process_t, node) : NULL);
}

mm_process_t *process_duplicate(mm_process_t *src, void *entry, void *stack)
{
	do {
		pid_t pid = process_find_new_pid();
		if (pid == -1)
			break;

		mm_process_t *dest;
		int error_no = process_create(&dest, pid);
		if (error_no)
			break;

		if (copy_user_pages(dest->directory, src->directory,
//TODO copy only current stack
				pageRoundUp(USER_STACK_END_ADDR)
//				pageRoundUp((uintptr_t) (src->segments.heap.addr)
//						+ src->segments.heap.len)
						>> BITS_OFFSET))
			//TODO release process
			break;

		dest->segments.exec = src->segments.exec;
		dest->segments.heap = src->segments.heap;
		dest->segments.stack = src->segments.stack;

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
			list_append(&((mm_process_group_t *) leader)->members,
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
		set_local(dest, src->local->wd, src->local->wd_len);

		log_info("d %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s %x\n",
				dest->node.key,
				&dest->brothers, dest->brothers.next,
				&dest->members, dest->members.next,
				dest->ppid, dest->pgid, dest->uid, dest->gid,
				dest->name,
				(dest->wd) ? dest->wd->node.key : (-1));

		error_no = map_user_stack(dest);
		if (error_no) {
			log_err("mm: th map err\n");
			//TODO release process
			break;
		}

		int thread_id;
		error_no = create_thread(&thread_id, dest,
				(FP) entry, (VP) stack);
		if (error_no) {
			//TODO release process
			break;
		}

		if (kcall->thread_start(thread_id) < 0) {
			log_err("mm: th start err\n");
			//TODO release process
			break;
		}

		return dest;
	} while (false);

	return NULL;
}

static int process_find_new_pid(void)
{
	for (int id = INIT_PID; id < PROCESS_MAX; id++)
		if (!tree_get(&process_tree, id))
			return id;

	return (-1);
}

int process_replace(mm_process_t *process,
		void *address, const size_t size,
		void *entry, const void *args, const size_t stack_size,
		int *thread_id)
{
	if (stack_size > USER_STACK_INITIAL_SIZE)
		return E2BIG;

	destroy_threads(process);

	if (map_user_stack(process))
		return ENOMEM;

	uintptr_t stack_top = pageRoundDown(USER_STACK_END_ADDR) - stack_size;
	if (move_stack(process->directory, (void *) stack_top,
			(void *) args, stack_size))
		return EFAULT;

	if (release_memory(process))
		return EFAULT;

	uintptr_t start = pageRoundDown((uintptr_t) address);
	uintptr_t end = pageRoundUp((uintptr_t) address + size);
	if (map_user_pages(process->directory,
			(void *) start, pages(end - start)))
		return ENOMEM;

	process->segments.exec.addr = (void *) start;
	process->segments.exec.len = end - start;
	process->segments.exec.max = process->segments.exec.len;
	process->segments.exec.attr = type_exec;

	process->segments.heap.addr = (void *) end;
	process->segments.heap.len = 0;
	process->segments.heap.max = pageRoundUp(USER_HEAP_MAX_ADDR) - end;
	process->segments.heap.attr = type_heap;

	process->segments.stack.max =
			THREADS_PER_PROCESS * USER_STACK_MAX_SIZE;
	process->segments.stack.addr = (void *) (
			(uintptr_t) USER_STACK_END_ADDR
			- process->segments.stack.max);
	process->segments.stack.len = 0;
	process->segments.stack.attr = type_heap;

	//TODO check if directory is current PDE
	tlb_flush_all();
	return create_thread(thread_id, process, (FP) entry,
			(VP) (stack_top - sizeof(int)));
}

int process_release_body(mm_process_t *proc, const int options)
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

		if ((proc->wpid != (-1))
				&& (proc->wpid != p->node.key))
			continue;

		int tag = proc->tag;
		sys_reply_t reply = {
			p->node.key,
			{ p->exit_status & 0xff }
		};

		proc->tag = 0;
		proc->wpid = 0;
		list_remove(&(p->members));
		list_remove(child);
		if (!tree_remove(&process_tree, p->node.key)) {
			//TODO what to do?
		}

		slab_free(&process_slab, p);

		ER result = kcall->ipc_send(tag, &reply, sizeof(reply));
		if (result)
			log_err("mm: %d failed to release body(%d)\n",
					proc->node.key, result);

		log_info("mm: %d success release body\n", proc->node.key);
		return 0;
	}

	if (options & WNOHANG) {
		int tag = proc->tag;
		proc->tag = 0;
		proc->wpid = 0;
		sys_reply_t reply = { 0, { 0 } };
		ER result = kcall->ipc_send(tag, &reply, sizeof(reply));
		if (result)
			log_err("mm: %d failed to release body(%d)\n",
					proc->node.key, result);
	}

	log_info("mm: %d not found release body\n", proc->node.key);
	return 0;
}

int process_destroy(mm_process_t *process, const int status)
{
	process->exit_status = status;
	process->session = NULL;
	destroy_threads(process);

	if (release_memory(process)) {
		//TODO what to do?
	}

	if (unmap_user_pages(process->directory, (void *) PROCESS_LOCAL_ADDR,
			pages(sizeof(*(process->local))))) {
		log_err("mm: unmap error %p,%x\n",
				(void *) PROCESS_LOCAL_ADDR,
				pages(sizeof(*(process->local))));

		//TODO what to do?
	}

	process->local = NULL;
	kcall->pfree(process->directory);
	process->directory = NULL;

	mm_process_t *parent = process_find(process->ppid);
	if (parent) {
		if (parent->tag) {
			log_info("mm: %d release parent %d\n",
					process->node.key, process->ppid);
			process_release_body(parent, 0);
		} else {
			log_info("mm: %d parent not waiting\n",
					process->node.key);
		}
	} else {
		//TODO what to do?
		log_info("mm: %d no parent\n", process->node.key);
	}

//TODO if parent process is 'init'?
//TODO if current process is 'init'?
	parent = process_find(INIT_PID);
	if (parent) {
		bool found = false;
		for (;;) {
			list_t *child = list_pick(&(process->children));
			if (!child)
				break;

			mm_process_t *p = getProcessFromBrothers(child);
			p->ppid = INIT_PID;
			list_append(&(parent->children), child);
			found = true;
		}

		if (found) {
			if (parent->tag)
				process_release_body(parent, 0);
		}
	} else {
		//TODO what to do?
	}

	return 0;
}

//TODO key must not be 0
int create_init(const pid_t pid, const FP entry)
{
	mm_process_t *p;
	int error_no = process_create(&p, pid);
	if (error_no)
		return error_no;

	if (map_user_pages(p->directory, (void *) PROCESS_LOCAL_ADDR,
			pages(sizeof(*(p->local)))))
		//TODO release process
		return ENOMEM;

	p->segments.exec.addr = (void *) 0;
	p->segments.exec.len = 0;
	p->segments.exec.max = 0;
	p->segments.exec.attr = type_exec;

	p->segments.heap.addr = (void *) 0;
	p->segments.heap.len = 0;
	p->segments.heap.max = 0;
	p->segments.heap.attr = type_heap;

	p->segments.stack.addr = (void *) 0;
	p->segments.stack.len = 0;
	p->segments.stack.max = 0;
	p->segments.stack.attr = type_stack;

	p->ppid = INIT_PPID;
	p->pgid = INIT_PID;
	p->uid = INIT_UID;
	p->gid = INIT_GID;
	p->session = NULL;
	p->wd = NULL;

	mm_process_group_t *pg = slab_alloc(&process_group_slab);
	if (pg) {
		list_initialize(&(pg->members));
		if (!tree_put(&process_group_tree, INIT_PID, (node_t *) pg)) {
			//TODO what to do?
			slab_free(&process_group_slab, pg);
		}
	}

	set_local(p, "/", 1);

	//TODO open cons
	mm_descriptor_t *d = process_create_dummy_file();
	if (d) {
		mm_file_t *f = d->file;
		f->server_id = PORT_CONSOLE;
		f->node.key = 0;
		f->f_flag = O_RDONLY;
		f->f_count = 1;
		f->f_offset = 0;
		if (process_set_desc(p, STDIN_FILENO, d)) {
			//TODO what to do?
			session_deallocate_file(f);
			process_deallocate_desc(d);
		}
	}

	//TODO create dummy session
	d = process_create_dummy_file();
	if (d) {
		mm_file_t *f = d->file;
		f->server_id = PORT_CONSOLE;
		f->node.key = 0;
		f->f_flag = O_WRONLY;
		f->f_count = 1;
		f->f_offset = 0;
		if (process_set_desc(p, STDOUT_FILENO, d)) {
			//TODO what to do?
			session_deallocate_file(f);
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
	error_no = create_init_thread(p, entry);
	if (error_no) {
		//TODO destroy process
		log_info("mm: create_init_thread %d\n", error_no);
		return error_no;
	}

	log_info("mm: create_init(pid=%d)\n", pid);
	return 0;
}

static int process_create(mm_process_t **process, const int pid)
{
	int error_no;
	do {
		mm_process_t *p = (mm_process_t *) slab_alloc(&process_slab);
		if (!p) {
			error_no = ENOMEM;
			break;
		}

		if (!tree_put(&process_tree, pid, (node_t *) p)) {
			slab_free(&process_slab, p);
			error_no = EBUSY;
			break;
		}

		p->segments.exec.attr = attr_nil;
		p->segments.heap.attr = attr_nil;
		p->segments.stack.attr = attr_nil;

		//TODO take over fields when duplicate?
		list_initialize(&(p->threads));
		//TODO leave on exec
		tree_create(&(p->descriptors), NULL, NULL);
		tree_create(&(p->sessions), NULL, NULL);
		list_initialize(&(p->brothers));
		list_initialize(&(p->children));
		list_initialize(&(p->members));
		p->local = NULL;
		p->tag = 0;
		p->wpid = 0;
		p->name[0] = '\0';

		//TODO check NULL
		p->directory = copy_kernel_page_table();
		*process = p;
		return 0;
	} while (false);

	return error_no;
}

static void set_local(mm_process_t *process, const char *wd,
		const size_t wd_len)
{
	process->local = getPageAddress(kern_p2v(process->directory),
			(void *) PROCESS_LOCAL_ADDR);
	memset(process->local, 0, sizeof(*(process->local)));
	process->local->pid = process->node.key;
	process->local->ppid = process->ppid;
	process->local->uid = process->uid;
	process->local->gid = process->gid;
	process->local->wd_len = wd_len;
	strcpy(process->local->wd, wd);
}

int mm_thread_find(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find((ID) (req->args.arg1));
		if (!th) {
			reply->data[0] = ESRCH;
			break;
		}

		reply->data[0] = 0;
		reply->result = ((mm_process_t *) (th->process))->node.key;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

mm_thread_t *thread_find(const ID tid)
{
	node_t *node = tree_get(&thread_tree, tid);
	return (node ? (mm_thread_t *) getParent(mm_thread_t, node) : NULL);
}

static int map_user_stack(mm_process_t *process)
{
	if (map_user_pages(process->directory,
			(void *) pageRoundDown(USER_STACK_END_ADDR
					- USER_STACK_INITIAL_SIZE),
			pages(pageRoundUp(USER_STACK_INITIAL_SIZE))))
		return ENOMEM;

	return 0;
}

static int release_memory(mm_process_t *process)
{
	if (unmap_user_pages(process->directory,
			process->segments.exec.addr,
			//TODO skip holes
			pages(process->segments.exec.len
					+  process->segments.heap.len))) {
		return EFAULT;
	}

	process->segments.exec.addr = (void *) 0;
	process->segments.exec.len = 0;
	process->segments.exec.max = 0;
	process->segments.exec.attr = attr_nil;

	process->segments.heap.addr = (void *) 0;
	process->segments.heap.len = 0;
	process->segments.heap.max = 0;
	process->segments.heap.attr = attr_nil;

	process->segments.stack.addr = (void *) 0;
	process->segments.stack.len = 0;
	process->segments.stack.max = 0;
	process->segments.stack.attr = attr_nil;

	return 0;
}

static int create_thread(int *thread_id, mm_process_t *process, FP entry,
		VP ustack_top)
{
	mm_thread_t *th = (mm_thread_t *) slab_alloc(&thread_slab);
	if (!th)
		return ENOMEM;

	T_CTSK pk_ctsk = {
		TA_HLNG,
		(VP_INT) NULL,
		entry,
		pri_user_foreground,
		KTHREAD_STACK_SIZE,
		NULL,
		process->directory,
		ustack_top
	};
	ER_ID tid = kcall->thread_create_auto(&pk_ctsk);
	if (tid < 0) {
		slab_free(&thread_slab, th);
		//TODO use another errno
		return ENOMEM;
	}

	if (!tree_put(&thread_tree, tid, &(th->node))) {
		kcall->thread_destroy(tid);
		slab_free(&thread_slab, th);
		//TODO use another errno
		return EBUSY;
	}

	list_append(&(process->threads), &(th->brothers));
	th->process = process;

	//TODO set process stack map
	th->stack.addr = (void *) pageRoundDown(
			(uintptr_t) USER_STACK_END_ADDR
			- USER_STACK_MAX_SIZE);
	th->stack.len = pageRoundUp(USER_STACK_INITIAL_SIZE);
	th->stack.max = pageRoundUp(USER_STACK_MAX_SIZE);
	th->stack.attr = type_stack;

	*thread_id = tid;

	thread_local_t *local = (thread_local_t *) MAIN_THREAD_LOCAL_ADDR;
	if (kcall->region_put(tid, &(local->thread_id), sizeof(tid), &tid)) {
		//TODO what to do?
	}

	return 0;
}

static void destroy_threads(mm_process_t *process)
{
	for (list_t *p; (p = list_pick(&(process->threads)));) {
		mm_thread_t *th = getMyThread(p);
		//TODO check error
		ER result = kcall->thread_terminate(th->node.key);
		if (result)
			log_err("mm: failed to terminate(%d) %d\n",
					th->node.key, result);
		//TODO check error
		result = kcall->thread_destroy(th->node.key);
		if (result)
			log_err("mm: failed to destroy(%d) %d\n",
					th->node.key, result);

		//TODO check error
		tree_remove(&thread_tree, th->node.key);

		uintptr_t start = (uintptr_t) th->stack.addr
				+ th->stack.max - th->stack.len;
		//TODO check error
		if (unmap_user_pages(process->directory,
				(void *) start, pages(th->stack.len)))
			log_err("mm: unmap error %p,%x\n",
					start, pages(th->stack.len));

		slab_free(&thread_slab, th);
	}
}

static int create_init_thread(mm_process_t *process, const FP entry)
{
	mm_thread_t *th = (mm_thread_t *) slab_alloc(&thread_slab);
	if (!th)
		return ENOMEM;

	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT) NULL,
		entry,
		pri_user_foreground,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
	ER_ID tid = kcall->thread_create_auto(&pk_ctsk);
	if (tid < 0) {
		slab_free(&thread_slab, th);
		//TODO use another errno
		return ENOMEM;
	}

	if (!tree_put(&thread_tree, tid, &(th->node))) {
		kcall->thread_destroy(tid);
		slab_free(&thread_slab, th);
		//TODO use another errno
		return EBUSY;
	}

	list_append(&(process->threads), &(th->brothers));
	th->process = process;
	return 0;
}
