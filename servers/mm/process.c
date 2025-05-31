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
#include <libc.h>
#include <mpufunc.h>
#include <mm/config.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "mm.h"
#include "api.h"
#include "process.h"

#define THREAD_LOOKUP_SIZE (16)

enum {
	SEQUENCE_SLAB = 0x01,
	PROCESS_SLAB = 0x02,
	PROCESS_SEQUENCE = 0x04,
	THREAD_SLAB = 0x08,
	THREAD_SEQUENCE = 0x10,
	SEGMENT_SLAB = 0x20,
	NAMESPACE_SLAB = 0x40
};

static sys_reply_t async_reply = { -1, { EFAULT, 0 } };
slab_t sequence_slab;
static slab_t process_slab;
static tree_t process_tree;
static sequence_t process_sequence;
static slab_t thread_slab;
static tree_t thread_tree;
static sequence_t thread_sequence;
static node_t *thread_lookup[THREAD_LOOKUP_SIZE];
static slab_t segment_slab;
static slab_t namespace_slab;
static int initialized_resources = 0;
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
*/

static node_t **thread_lookup_selector(const tree_t *, const int);
static int process_create(mm_process_t **, const int);
static void set_local(mm_process_t *, const char *, const size_t);
static int map_user_stack(mm_process_t *);
static int release_memory(mm_process_t *);
static int _segment_share(void *, mm_segment_t **, const void *,
	mm_segment_t *);
static int _segment_copy(void *, mm_segment_t **, const void *,
	mm_segment_t *);
static int _segment_allocate(mm_segment_t **, void *, const void *,
	const void *, const int, const unsigned int);
static void _segment_deallocate(mm_process_t *, mm_segment_t *);
static int _convert_permission(const mm_segment_t *);
static int create_thread(const int, mm_process_t *, const void *, const void *);
static void destroy_threads(mm_process_t *, const int);
static int create_spawn_thread(mm_process_t *, const FP);
static int _thread_create_frame(mm_thread_t **, T_CTSK *, const int,
		mm_process_t *);


static node_t **thread_lookup_selector(const tree_t *tree, const int key)
{
	return &(((node_t **) tree->root)[key & (THREAD_LOOKUP_SIZE - 1)]);
}

int process_initialize(void)
{
	if (create_slab(&sequence_slab, SEQUENCE_MAP_SIZE(PROCESS_MAX),
			PROCESS_MAX * 2))
		return SEQUENCE_SLAB;
	else
		initialized_resources |= SEQUENCE_SLAB;

	if (create_slab(&process_slab, sizeof(mm_process_t), PROCESS_MAX))
		return PROCESS_SLAB;
	else
		initialized_resources |= PROCESS_SLAB;

	tree_create(&process_tree, NULL, NULL);

	if (create_id_store(&process_sequence, PROCESS_MAX))
		return PROCESS_SEQUENCE;
	else
		initialized_resources |= PROCESS_SEQUENCE;

	//TODO ugly. skip 1 as process id
	if (sequence_get(&process_sequence) < 0)
		return PROCESS_SEQUENCE;

	if (create_slab(&thread_slab, sizeof(mm_thread_t), THREAD_MAX))
		return THREAD_SLAB;
	else
		initialized_resources |= THREAD_SLAB;

	tree_create(&thread_tree, NULL, thread_lookup_selector);
	tree_initialize_lookup(&thread_tree, thread_lookup, THREAD_LOOKUP_SIZE);

	if (create_id_store(&thread_sequence, THREAD_MAX))
		return THREAD_SEQUENCE;
	else
		initialized_resources |= THREAD_SEQUENCE;

	if (create_slab(&segment_slab, sizeof(mm_segment_t), SEGMENT_MAX))
		return SEGMENT_SLAB;
	else
		initialized_resources |= SEGMENT_SLAB;

	if (create_slab(&namespace_slab, sizeof(mm_namespace_t), NAMESPACE_MAX))
		return NAMESPACE_SLAB;
	else
		initialized_resources |= NAMESPACE_SLAB;

	return 0;
}

mm_process_t *process_find(const ID pid)
{
	node_t *node = tree_get(&process_tree, pid);
	return (node ? (mm_process_t *) getParent(mm_process_t, node) : NULL);
}

//TODO release resources when error
mm_process_t *process_duplicate(const mm_thread_t *th, const int flags,
		const void *entry, const void *stack)
{
	do {
		pid_t pid = sequence_get(&process_sequence);
		if (pid == -1)
			break;

		mm_process_t *dest;
		int error_no = process_create(&dest, pid);
		if (error_no)
			break;

		// exec
		mm_process_t *src = get_process(th);
		error_no = _segment_share(dest->directory,
				&(dest->segments.exec), src->directory,
				src->segments.exec);
		if (error_no)
			break;

		// data
		if (src->segments.data) {
			if (flags & RFMEM) {
				error_no = _segment_share(
						dest->directory,
						&(dest->segments.data),
						src->directory,
						src->segments.data);
				if (error_no)
					break;
			} else {
				error_no = _segment_copy(
						dest->directory,
						&(dest->segments.data),
						src->directory,
						src->segments.data);
				if (error_no)
					break;
			}
		}

		// heap
		if (src->segments.heap) {
			if (flags & RFMEM) {
				error_no = _segment_share(
						dest->directory,
						&(dest->segments.heap),
						src->directory,
						src->segments.heap);
				if (error_no)
					break;
			} else {
				error_no = _segment_copy(
						dest->directory,
						&(dest->segments.heap),
						src->directory,
						src->segments.heap);
				if (error_no)
					break;
			}
		}

		// process local
		error_no = copy_user_pages(dest->directory, src->directory,
				(void *) PROCESS_LOCAL_ADDR,
				pages(sizeof(*(src->local))), R_OK);
		if (error_no) {
			log_warning("failed to copy local %d\n", error_no);
			break;
		}

		// stack
		error_no = copy_user_pages(dest->directory, src->directory,
				(void *) ((uintptr_t) (th->stack.addr)
						- th->stack.len
						+ th->stack.max),
				pages(th->stack.len), R_OK | W_OK);
		if (error_no) {
			log_warning("failed to copy stack %d\n", error_no);
			break;
		}

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

		if (!(flags & RFNOTEG)) {
			mm_process_t *leader = process_find(src->pgid);
			if (leader)
				list_append(&(leader->members),
						&(dest->members));
		}

		for (list_t *p = src->namespaces.next;
				!list_is_edge(&(src->namespaces), p);
				p = p->next) {
			mm_namespace_t *s = getNamespaceFromBrothers(p);
			mm_namespace_t *d = process_allocate_ns();
			if (!d) {
				log_err("mm: ns alloc err\n");
				//TODO release process
				break;
			}

			d->root = s->root;
			strcpy(d->name, s->name);
			list_append(&(dest->namespaces), &(d->brothers));
			d->root->f_count++;
		}

		dest->ppid = src->node.key;
		dest->pgid = (flags & RFNOTEG) ? pid : (src->pgid);
		dest->uid = src->uid;
		dest->gid = src->gid;
		dest->root = src->root;
		if (dest->root)
			dest->root->f_count++;

		set_local(dest, src->local->wd, src->local->wd_len);
		strcpy(dest->local->name, src->local->name);

		int thread_id = sequence_get(&thread_sequence);
		if (thread_id < 0) {
			log_err("mm: th sequence exhausted\n");
			//TODO release process
			break;
		}

		thread_id += INIT_THREAD_ID;
		error_no = create_thread(thread_id, dest, entry, stack);
		if (error_no) {
			//TODO release process
			sequence_release(&thread_sequence,
					thread_id - INIT_THREAD_ID);
			break;
		}

		if (kcall->thread_start(thread_id) < 0) {
			log_err("mm: th start err\n");
			//TODO release process
			sequence_release(&thread_sequence,
					thread_id - INIT_THREAD_ID);
			break;
		}

		return dest;
	} while (false);

	return NULL;
}

//TODO release resources when error
int process_replace(mm_thread_t *th, const Elf32_Phdr *code,
		const Elf32_Phdr *data, void *entry, const void *args,
		const size_t stack_size)
{
	//TODO support ARG_MAX > USER_STACK_INITIAL_SIZE
	if (stack_size > USER_STACK_INITIAL_SIZE)
		return E2BIG;

	int thread_id = th->node.key;
	mm_process_t *process = get_process(th);
	destroy_threads(process, thread_id);

	if (map_user_stack(process))
		return ENOMEM;

	uintptr_t stack_top = pageRoundDown(USER_STACK_END_ADDR) - stack_size;
	if (move_stack(process->directory, (void *) stack_top,
			(void *) args, stack_size))
		return EFAULT;

	if (release_memory(process))
		return EFAULT;

	uintptr_t start = pageRoundDown(code->p_vaddr);
	uintptr_t end = pageRoundUp(code->p_vaddr + code->p_memsz);
	int result = _segment_allocate(&(process->segments.exec),
			process->directory, (void *) start, (void *) end,
			R_OK | X_OK, type_code);
	if (result)
		return result;

	if (data->p_memsz) {
		start = pageRoundDown(data->p_vaddr);
		end = pageRoundUp(data->p_vaddr + data->p_memsz);
		result = _segment_allocate(&(process->segments.data),
			process->directory, (void *) start, (void *) end,
			R_OK | W_OK, type_data);
		if (result)
			return result;
	}

	//TODO check if directory is current PDE
	tlb_flush_all();
	return create_thread(thread_id, process, entry,
			(void *) (stack_top - sizeof(int)));
}

int process_release_body(mm_process_t *proc, const int options)
{
//TODO list_insert dead child on exit
	if (list_is_empty(&(proc->children))) {
		//log_info("mm: %d no child release body\n", proc->node.key);
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

		sequence_release(&process_sequence, p->node.key);

		//log_info("mm: %d released %d\n", proc->node.key, p->node.key);
		slab_free(&process_slab, p);

		ER result = kcall->ipc_send(tag, &reply, sizeof(reply));
		if (result)
			log_err("mm: %d failed to release body(%d)\n",
					proc->node.key, result);

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

	//log_info("mm: %d not found release body\n", proc->node.key);
	return 0;
}

int process_destroy(mm_process_t *process, const int status)
{
	process->exit_status = status;
	destroy_threads(process, 0);

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
			//log_info("mm: %d release parent %d\n",
			//		process->node.key, process->ppid);
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
int spawn(const pid_t pid, const FP entry)
{
	mm_process_t *p;
	int error_no = process_create(&p, pid);
	if (error_no)
		return error_no;

	if (map_user_pages(p->directory, (void *) PROCESS_LOCAL_ADDR,
			pages(sizeof(*(p->local))), R_OK))
		//TODO release process
		return ENOMEM;

	p->ppid = INIT_PPID;
	p->pgid = INIT_PID;
	p->uid = INIT_UID;
	p->gid = INIT_GID;
	p->root = NULL;

	set_local(p, "/", 1);
#if 0
	log_info("c %d %x->%x, %x->%x pp=%d pg=%d u=%d g=%d n=%s\n",
			p->node.key,
			&p->brothers, p->brothers.next,
			&p->members, p->members.next,
			p->ppid, p->pgid, p->uid, p->gid,
			p->name);
#endif
	error_no = create_spawn_thread(p, entry);
	if (error_no) {
		//TODO destroy process
		log_info("mm: create_spawn_thread %d\n", error_no);
		return error_no;
	}

	error_no = kcall->thread_start(INIT_THREAD_ID);
	if (error_no) {
		//TODO destroy process
		log_info("mm: failed to start thread %d\n", error_no);
		return EBUSY;
	}

	log_info("mm: spawn(pid=%d)\n", pid);
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

		p->segments.exec = NULL;
		p->segments.data = NULL;
		p->segments.heap = NULL;

		//TODO take over fields when duplicate?
		list_initialize(&(p->threads));
		//TODO leave on exec
		tree_create(&(p->descriptors), NULL, NULL);
		list_initialize(&(p->brothers));
		list_initialize(&(p->children));
		list_initialize(&(p->members));
		list_initialize(&(p->namespaces));
		p->local = NULL;
		p->tag = 0;
		p->wpid = 0;

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
	process->local->name[0] = '\0';
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
			pages(pageRoundUp(USER_STACK_INITIAL_SIZE)),
			R_OK | W_OK))
		return ENOMEM;

	return 0;
}

static int release_memory(mm_process_t *process)
{
	if (process->segments.exec) {
		_segment_deallocate(process, process->segments.exec);
		process->segments.exec = NULL;
	}

	if (process->segments.data) {
		_segment_deallocate(process, process->segments.data);
		process->segments.data = NULL;
	}

	if (process->segments.heap) {
		_segment_deallocate(process, process->segments.heap);
		process->segments.heap = NULL;
	}

	return 0;
}

static int _segment_share(void *dest_directory, mm_segment_t **dest,
		const void *src_directory, mm_segment_t *src)
{
	int error_no = attach_user_pages(dest_directory, src_directory,
			src->addr, pages(src->len), _convert_permission(src));
	if (error_no) {
		log_warning("failed to share(%p, %x) %d\n", src->addr, src->len,
				error_no);
		//TODO detach for recovery
		return error_no;
	}

	src->ref_count++;
	*dest = src;
	return 0;
}

static int _segment_copy(void *dest_directory, mm_segment_t **dest,
		const void *src_directory, mm_segment_t *src)
{
	mm_segment_t *s = (mm_segment_t *) slab_alloc(&segment_slab);
	if (!s) {
		log_warning("failed to allocate segment\n");
		return ENOMEM;
	}

	int error_no = copy_user_pages(dest_directory, src_directory,
			src->addr, pages(src->len), _convert_permission(src));
	if (error_no) {
		log_warning("failed to copy(%p, %x) %d\n", src->addr, src->len,
				error_no);
		slab_free(&segment_slab, s);
		//TODO unmap for recovery
		return error_no;
	}

	s->addr = src->addr;
	s->len = src->len;
	s->max = src->max;
	s->attr = src->attr;
	s->ref_count = 1;
	list_initialize(&(s->semaphores));
	*dest = s;
	return 0;
}

static int _segment_allocate(mm_segment_t **dest, void *directory,
		const void *start, const void *end, const int attr,
		const unsigned int type)
{
	mm_segment_t *s = (mm_segment_t *) slab_alloc(&segment_slab);
	if (!s) {
		log_warning("failed to allocate segment\n");
		return ENOMEM;
	}

	int error_no = map_user_pages(directory, (void *) start,
			pages((uintptr_t) end - (uintptr_t) start), attr);
	if (error_no) {
		log_warning("failed to map(%p, %x) %d\n", start,
				(uintptr_t) end - (uintptr_t) start, error_no);
		slab_free(&segment_slab, s);
		//TODO unmap for recovery
		return error_no;
	}

	s->addr = (void *) start;
	s->max = s->len = (uintptr_t) end - (uintptr_t) start;
	s->attr = type;
	s->ref_count = 1;
	list_initialize(&(s->semaphores));
	*dest = s;
	return 0;
}

static void _segment_deallocate(mm_process_t *process, mm_segment_t *s)
{
	if (s->ref_count > 1) {
		int result = detach_user_pages(process->directory, s->addr,
				pages(s->len));
		if (result)
			log_warning("failed to detach(%p, %x) %d\n", s->addr,
				s->len, result);

		s->ref_count--;
	} else {
		int result = unmap_user_pages(process->directory, s->addr,
				pages(s->len));
		if (result)
			log_warning("failed to unmap(%p, %x) %d\n", s->addr,
				s->len, result);

		for (list_t *p; (p = list_pick(&(s->semaphores)));)
			semaphore_destroy(getSemaphoreFromSegments(p));

		slab_free(&segment_slab, s);
	}
}

int process_allocate_heap(mm_process_t *p)
{
	mm_segment_t *s = (mm_segment_t *) slab_alloc(&segment_slab);
	if (!s)
		return ENOMEM;

	mm_segment_t *prev = (p->segments.data) ?
			(p->segments.data) : (p->segments.exec);
	uintptr_t end = ((uintptr_t) prev->addr) + prev->len;
	s->addr = (void *) end;
	s->len = 0;
	s->max = pageRoundUp(USER_HEAP_MAX_ADDR) - end;
	s->attr = type_heap;
	s->ref_count = 1;
	list_initialize(&(s->semaphores));
	p->segments.heap = s;
	return 0;
}

static int _convert_permission(const mm_segment_t *s)
{
	int permission = (s->attr & attr_readable) ? R_OK : 0;
	if (s->attr & attr_writable)
		permission |= W_OK;

	return permission;
}

static int create_thread(const int thread_id, mm_process_t *process,
		const void *entry, const void *ustack_top)
{
	T_CTSK pk_ctsk = {
		TA_HLNG,
		(VP_INT) NULL,
		entry,
		pri_user_foreground,
		KTHREAD_STACK_SIZE,
		NULL,
		process->directory,
		(VP) ustack_top
	};
	mm_thread_t *th;
	int result = _thread_create_frame(&th, &pk_ctsk, thread_id, process);
	if (result)
		return result;

	//TODO set process stack map
	th->stack.addr = (void *) pageRoundDown(
			(uintptr_t) USER_STACK_END_ADDR
			- USER_STACK_MAX_SIZE);
	th->stack.len = USER_STACK_END_ADDR
			- pageRoundDown((uintptr_t ) ustack_top);
	if (th->stack.len < pageRoundUp(USER_STACK_INITIAL_SIZE))
		th->stack.len = pageRoundUp(USER_STACK_INITIAL_SIZE);

	th->stack.max = pageRoundUp(USER_STACK_MAX_SIZE);
	th->stack.attr = type_stack;
	th->stack.ref_count = 1;
	list_initialize(&(th->stack.semaphores));
	return 0;
}

static void destroy_threads(mm_process_t *process, const int thread_id)
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

		if (th->node.key != thread_id)
			sequence_release(&thread_sequence,
					th->node.key - INIT_THREAD_ID);

		if (th->stack.attr) {
			uintptr_t start = (uintptr_t) th->stack.addr
					+ th->stack.max - th->stack.len;
			result = unmap_user_pages(process->directory,
					(void *) start, pages(th->stack.len));
			if (result)
				log_warning("mm: failed to unmap(%p,%x) %d\n",
						start, pages(th->stack.len),
						result);
		}

		if (th->wait.reason) {
			list_remove(&(th->waiting));
			kcall->ipc_send(th->wait.tag, &async_reply,
					sizeof(async_reply));
		}

		slab_free(&thread_slab, th);
	}
}

static int create_spawn_thread(mm_process_t *process, const FP entry)
{
	T_CTSK pk_ctsk = {
		TA_HLNG,
		(VP_INT) NULL,
		entry,
		pri_user_foreground,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
	mm_thread_t *th;
	return _thread_create_frame(&th, &pk_ctsk, INIT_THREAD_ID, process);
}

static int _thread_create_frame(mm_thread_t **p, T_CTSK *pk_ctsk,
		const int thread_id, mm_process_t *process)
{
	mm_thread_t *th = (mm_thread_t *) slab_alloc(&thread_slab);
	if (!th)
		return ENOMEM;

	int result = kcall->thread_create(thread_id, pk_ctsk);
	if (result) {
		slab_free(&thread_slab, th);
		//TODO use adequate errno
		return ENOMEM;
	}

	if (!tree_put(&thread_tree, thread_id, &(th->node))) {
		result = kcall->thread_destroy(thread_id);
		if (result)
			log_warning("mm: failed to destroy(%d) %d\n",
					thread_id, result);

		slab_free(&thread_slab, th);
		//TODO use adequate errno
		return EBUSY;
	}

	list_initialize(&(th->waiting));
	th->wait.obj = 0;
	th->wait.reason = REASON_NONE;
	th->wait.tag = 0;
	th->process = process;
	th->stack.attr = attr_nil;

	list_append(&(process->threads), &(th->brothers));
	*p = th;
	return 0;
}

mm_namespace_t *process_allocate_ns(void)
{
	return (mm_namespace_t *) slab_alloc(&namespace_slab);
}

void process_deallocate_ns(mm_namespace_t *ns)
{
	slab_free(&namespace_slab, ns);
}

int create_slab(slab_t *slab, const size_t unit_size, const size_t max_unit)
{
	slab->unit_size = unit_size;
	slab->block_size = PAGE_SIZE;
	slab->min_block = 1;
	slab->max_block = slab_max_block(max_unit, PAGE_SIZE, unit_size);
	slab->palloc = kcall->palloc;
	slab->pfree = kcall->pfree;
	return slab_create(slab);
}

int create_id_store(sequence_t *seq, const size_t max)
{
	if (max > PROCESS_MAX)
		return (-1);

	void *buf = slab_alloc(&sequence_slab);
	if (!buf)
		return (-1);

	do {
		if (sequence_initialize(seq, max, buf))
			break;

		// skip 0
		if (sequence_get(seq) < 0)
			break;

		return 0;
	} while (false);

	slab_free(&sequence_slab, buf);
	return (-1);
}
