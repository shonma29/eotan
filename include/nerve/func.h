#ifndef _NERVE_FUNC_H_
#define _NERVE_FUNC_H_
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
#include <nerve/config.h>
#include <nerve/sync.h>
#include <nerve/thread.h>
#include <set/lf_queue.h>
#include <set/list.h>
#include <set/tree.h>

#define SHOW_WARN 0

#if SHOW_WARN
#define warn printk
#else
#define warn(...)
#endif

/* global.c */
extern void kcall_initialize(void);

/* palloc.c */
extern void *palloc(void);
extern void pfree(void *);

/* printk.c */
extern void printk(const char *, ...);

/* tree_utils.c */
extern void create_tree(tree_t *, slab_t *, size_t,
		int (*)(const int, const int));

/* thread.c */
extern thread_t *get_thread_ptr(ID);
extern ER thread_create(ID, T_CTSK *);
extern ER thread_destroy(ID);
extern void thread_end(void);
extern void thread_end_and_destroy(void);
extern ER thread_initialize(void);
extern ER thread_start(ID);
extern ER thread_terminate(ID);
extern ER thread_suspend(ID);
extern ER thread_resume(ID);
extern ID thread_get_id(void);

static inline int is_kthread(const thread_t *thread)
{
	return (thread->attr.ustack_top == NULL);
}

/* ipc.c */
extern int ipc_initialize(void);
extern int ipc_open(const T_CPOR *);
extern int ipc_close(void);
extern int ipc_call(const int, void *, const size_t);
extern int ipc_receive(const int, int *, void *);
extern int ipc_send(const int, const void *, const size_t);
extern int ipc_listen(void);
extern int ipc_notify(const int, const unsigned int);

/* mutex.c */
extern ER mutex_create(ID, T_CMTX *);
extern ER mutex_destroy(ID);
extern ER mutex_initialize(void);
extern ER mutex_lock(ID, TMO);
extern ER mutex_unlock(ID);
extern void mutex_unlock_all(thread_t *);

/* start.c */
extern void kern_start(void (*)(void));

#endif
