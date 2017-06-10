#ifndef _CORE_FUNC_H_
#define _CORE_FUNC_H_
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
#include <set/lf_queue.h>
#include <set/list.h>
#include <set/tree.h>
#include "sync.h"
#include "thread.h"

/* modules.c */
extern void load_modules(void);

/* global.c */
extern void kcall_initialize(void);

/* palloc.c */
extern void *palloc(void);
extern void pfree(void *addr);

/* printk.c */
extern void printk(const char *format, ...);

/* tree_utils.c */
extern void create_tree(tree_t *, slab_t *, size_t,
		int (*)(const int, const int));
extern node_t *find_empty_key(tree_t *, int *, node_t *);

/* thread.c */
extern thread_t *get_thread_ptr(ID);
extern ER_ID thread_create_auto(T_CTSK *);
extern ER thread_create(ID, T_CTSK *);
extern void thread_reset(thread_t *);
extern ER thread_destroy(ID);
extern void thread_end(void);
extern void thread_end_and_destroy(void);
extern ER thread_initialize(void);
extern ER thread_start(ID);
extern ER thread_terminate(ID);
extern ER thread_sleep(void);
extern ER thread_wakeup(ID);
extern ID thread_get_id(void);

static inline int is_kthread(const thread_t *th)
{
	return th->attr.page_table == NULL;
}

/* rendezvous.c */
extern ER_UINT port_accept(ID, RDVNO *, VP);
extern ER_UINT port_call(ID, VP, UINT);
extern ER port_open(T_CPOR *);
extern ER port_close(void);
extern ER port_initialize(void);
extern ER port_reply(RDVNO, VP, UINT);

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
