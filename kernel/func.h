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
#include <kthread.h>
#include <set/lf_queue.h>
#include <set/list.h>
#include <set/tree.h>
#include <sys/time.h>
#include "thread.h"


/* initialize.c */
extern ER core_initialize(void);

/* api.c */
extern void api_initialize(void);
extern W syscall (W *arg_addr, UW sysno);

/* modules.c */
extern void run_init_program(void);

/* kcall.c */
extern void kcall_initialize(void);

/* palloc.c */
extern void *palloc(size_t size);
extern void pfree(void *addr, size_t size);
extern UW pmemfree(void);

/* printk.c */
extern int printk(const char *format, ...);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* tree_utils.c */
extern node_t *find_empty_key(tree_t *tree, int *hand);

/* ready.c */
extern list_t *ready_dequeue();
extern void ready_enqueue(const int pri, list_t *src);
extern void ready_initialize();
extern void ready_rotate(const int pri);

/* sync.c */
extern void release(thread_t *task);
extern void wait(thread_t *task);

/* thread.c */
extern thread_t *run_task;
extern ER rot_rdq(PRI tskpri);
extern ER thread_change_priority(ID tskid, PRI tskpri);
extern ER thread_get_id(ID * p_tskid);
extern ER thread_release(ID tskid);
extern ER thread_resume(ID tskid);
extern ER thread_suspend(ID tskid);
extern ER thread_switch(void);

/* nthread.c */
extern thread_t *get_thread_ptr(ID tskid);
extern ER_ID idle_initialize(void);
extern ER_ID thread_create_auto(T_CTSK *pk_ctsk);
extern ER thread_destroy(ID tskid);
extern void thread_end(void);
extern void thread_end_and_destroy(void);
extern ER thread_initialize(void);
extern ER thread_start(ID tskid);
extern ER thread_terminate(ID tskid);

/* rendezvous.c */
extern ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg);
extern ER_UINT port_call(ID porid, VP msg, UINT cmsgsz);
extern ER port_create(ID porid, T_CPOR *pk_cpor);
extern ER_ID port_create_auto(T_CPOR *pk_cpor);
extern ER port_destroy(ID porid);
extern ER port_initialize(void);
extern ER port_reply(RDVNO rdvno, VP msg, UINT rmsgsz);

/* queue.c */
extern ER_ID queue_create_auto(T_CDTQ *pk_cdtq);
extern ER queue_destroy(ID dtqid);
extern ER queue_initialize(void);
extern ER queue_receive(ID dtqid, VP_INT *p_data);
extern ER queue_send(ID dtqid, VP_INT data);

/* time.c */
extern BOOL do_timer;
extern void check_timer(void);
extern void intr_interval(void);
extern ER thread_delay(RELTIM dlytim);
extern void timer_initialize(void);

/* ntime.c */
extern ER time_get(SYSTIM *pk_systim);
extern void time_initialize(time_t *seconds);
extern ER time_set(SYSTIM *pk_systim);
extern void time_tick(void);

/* delay.c */
extern volatile lfq_t kqueue;
extern kthread_t delay_thread;
extern ID delay_thread_id;
extern void start(void);

#endif
