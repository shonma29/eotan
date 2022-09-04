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
#include <interrupt.h>
#include <local.h>
#include <fs/protocol.h>
#include <mm/segment.h>
#include <mpu/memory.h>
#include <set/list.h>
#include <set/tree.h>
#include <sys/types.h>
#include "api.h"

#define MYNAME "mm"

typedef struct {
	node_t node;
	int server_id;
	int f_flag;
	int f_count;
	off_t f_offset;
} mm_file_t;

typedef struct {
	node_t node;
	mm_file_t *file;
} mm_descriptor_t;

typedef struct {
	node_t node;
	void *process;
	mm_segment_t stack;
	list_t brothers;
} mm_thread_t;

typedef struct {
	node_t node;
	tree_t files;
	int refer_count;
} mm_session_t;

typedef struct {
	node_t node;
	list_t members;
} mm_process_group_t;

typedef struct {
	node_t node;
	struct {
		mm_segment_t exec;
		mm_segment_t heap;
		mm_segment_t stack;
	} segments;
	void *directory;
	list_t threads;
	tree_t descriptors;
	tree_t sessions;//TODO attach on fork/walk
	list_t brothers;
	list_t children;
	list_t members;
	pid_t ppid;
	pid_t pgid;
	uid_t uid;
	gid_t gid;
	mm_session_t *session;
	mm_file_t *wd;
	process_local_t *local;
	int exit_status;
	int tag;
	pid_t wpid;
	char name[32];//TODO set on fork/exec
} mm_process_t;

static inline mm_process_t *get_process(const mm_thread_t *thread)
{
	return ((mm_process_t *) (thread->process));
}

extern void process_initialize(void);
extern mm_process_t *process_find(const ID);
extern mm_process_t *process_duplicate(mm_process_t *, void *, void *);
extern int process_replace(mm_process_t *process, void *address,
		const size_t size, void *entry, const void *args,
		const size_t stack_size, int *);
extern int process_release_body(mm_process_t *, const int);
extern int process_destroy(mm_process_t *, const int);
extern int create_init(const pid_t, const FP);
extern mm_thread_t *thread_find(const ID);

extern void file_initialize(void);
extern mm_session_t *session_create(void);
extern int session_destroy(mm_session_t *);
extern mm_descriptor_t *process_create_dummy_file(void);
extern mm_descriptor_t *process_create_desc(mm_process_t *);
extern int process_destroy_desc(mm_process_t *, const int);
extern int process_set_desc(mm_process_t *, const int, mm_descriptor_t *);
extern mm_descriptor_t *process_find_desc(const mm_process_t *, const int);
extern mm_descriptor_t *process_allocate_desc(void);
extern void process_deallocate_desc(mm_descriptor_t *);
extern mm_file_t *session_create_file(mm_session_t *);
extern int session_destroy_file(mm_session_t *, mm_file_t *);
extern void session_deallocate_file(mm_file_t *);

extern void init(void);

extern void default_handler(VP_INT);
extern void page_fault_handler(VP_INT);

#endif
