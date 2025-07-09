#ifndef _MM_PROCESS_H_
#define _MM_PROCESS_H_
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
#include <elf.h>
#include <interrupt.h>
#include <local.h>
#include <fs/protocol.h>
#include <mm/segment.h>
#include <mpu/memory.h>
#include <set/list.h>
#include <set/tree.h>
#include <set/sequence.h>
#include <sys/types.h>
#include "api.h"
#include "semaphore.h"

#define MYNAME "mm"

#define SIZE_NAMESPACE (8)

typedef enum {
	REASON_NONE = 0,
	REASON_SEMAPHORE = 1
} mm_waiting_reason_e;

typedef struct {
	node_t node;
	int server_id;
	tree_t files;
} mm_session_t;

typedef struct {
	node_t node;
	mm_session_t *session;
	off_t f_offset;
	int f_flag;
	int f_count;
} mm_file_t;

typedef struct {
	node_t node;
	mm_file_t *file;
} mm_descriptor_t;

typedef struct {
	list_t brothers;
	mm_file_t *root;
	char name[SIZE_NAMESPACE];
} mm_namespace_t;

typedef struct {
	node_t node;
	mm_segment_t stack;
	list_t brothers;
	list_t waiting;
	struct {
		uintptr_t obj;
		mm_waiting_reason_e reason;
		int tag;
	} wait;
	void *process;
} mm_thread_t;

typedef struct {
	node_t node;
	struct {
		mm_segment_t *code;
		mm_segment_t *data;
		mm_segment_t *heap;
	} segments;
	void *directory;
	list_t threads;
	tree_t descriptors;
	list_t brothers;
	list_t children;
	list_t members;
	list_t namespaces;
	pid_t ppid;
	pid_t pgid;
	uid_t uid;
	gid_t gid;
	mm_file_t *root;
	process_local_t *local;
	int status;
	int exit_status;
	int tag;
	pid_t wpid;
} mm_process_t;

extern slab_t sequence_slab;

//TODO check process status
//TODO check if accessible (user)
static inline mm_process_t *get_process(const mm_thread_t *thread)
{
	return ((mm_process_t *) (thread->process));
}

static inline mm_thread_t *getMyThread(const list_t *p)
{
	return ((mm_thread_t *) ((uintptr_t) p
			- offsetof(mm_thread_t, brothers)));
}

static inline mm_namespace_t *getNamespaceFromBrothers(const list_t *p)
{
	return ((mm_namespace_t *) ((uintptr_t) p
			- offsetof(mm_namespace_t, brothers)));
}

static inline int create_sid(const int server_id, const pid_t pid)
{
	return ((server_id << 16) | pid);
}

static inline int server_id_from_sid(const int sid)
{
	return ((sid >> 16) & 0xffff);
}

static inline int sequence_from_sid(const int sid)
{
	return (sid & 0xffff);
}

extern int process_initialize(void);
extern mm_process_t *process_find(const ID);
extern int process_duplicate(mm_process_t ** const, const mm_thread_t *,
		const int, const void *, const void *);
extern int process_replace(mm_thread_t *, const Elf32_Phdr *,
		const Elf32_Phdr *, void *entry, const void *args,
		const size_t stack_size);
extern int process_release_body(mm_process_t *, const int);
extern void process_destroy(mm_process_t *, const int);
extern int spawn(const pid_t, const FP);
extern mm_thread_t *thread_find(const ID);

extern int file_initialize(void);
extern mm_session_t *session_create(const int);
extern int session_destroy(mm_session_t *);
extern mm_descriptor_t *process_create_desc(mm_process_t *);
extern int process_destroy_desc(mm_process_t *, const int);
extern int process_set_desc(mm_process_t *, const int, mm_descriptor_t *);
extern mm_descriptor_t *process_find_desc(const mm_process_t *, const int);
extern mm_descriptor_t *process_allocate_desc(void);
extern void process_deallocate_desc(mm_descriptor_t *);
extern mm_file_t *session_create_file(mm_session_t *);
extern int session_destroy_file(mm_file_t *);
extern int process_allocate_heap(mm_process_t *);
extern mm_namespace_t *process_allocate_ns(void);
extern void process_deallocate_ns(mm_namespace_t *);

extern void load(void);

extern void default_handler(VP_INT);
extern void page_fault_handler(VP_INT);

extern int create_slab(slab_t *, const size_t, const size_t);
extern int create_id_store(sequence_t *, const size_t);

#endif
