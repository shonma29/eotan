#ifndef _PROCESS_H_
#define _PROCESS_H_
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
#include <local.h>
#include <pm.h>
#include <mm/segment.h>
#include <mpu/memory.h>
#include <set/list.h>
#include <set/tree.h>
#include <sys/types.h>

typedef struct {
	int server_id;
	int fid;
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
//TODO pointer to process
	int process_id;
	mm_segment_t stack;
	list_t brothers;
} mm_thread_t;

typedef struct {
	node_t node;
	list_t members;
} mm_process_group_t;

typedef struct {
	node_t node;
	int fid;
} mm_session_t;

typedef struct {
	node_t node;
	struct {
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
	int session_id;
	thread_local_t *local;
	int exit_status;
	RDVNO rdvno;
	char name[32];//TODO set on fork/exec
} mm_process_t;

extern void process_initialize(void);
extern mm_process_t *get_process(const ID);
extern mm_thread_t *get_thread(const ID);
extern mm_process_t *process_duplicate(mm_process_t *);
extern int process_destroy(mm_process_t *, const int);
extern int process_release_body(mm_process_t *);
extern int thread_create(mm_process_t *, FP, VP);
extern mm_descriptor_t *process_create_file(void);
extern void process_deallocate_file(mm_file_t *);
extern mm_descriptor_t *process_allocate_desc(void);
extern void process_deallocate_desc(mm_descriptor_t *);
extern int process_set_desc(mm_process_t *, const int, mm_descriptor_t *);
extern int process_destroy_desc(mm_process_t *, const int);
extern mm_descriptor_t *process_find_desc(const mm_process_t *, const int);
int process_find_new_fd(const mm_process_t *);

extern ER default_handler(void);
extern ER stack_fault_handler(void);

extern int if_exit(mm_process_t *, pm_args_t *);

#endif
