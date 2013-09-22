/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* func.h - 大域関数の定義
 *
 */

#ifndef __CORE_FUNC_H__
#define __CORE_FUNC_H__	1


#include <core.h>
#include <set/lf_queue.h>
#include <set/tree.h>
#include <mpu/memory.h>
#include "thread.h"

/* main.c */
extern volatile lfq_t kqueue;
extern int main(void);

/* printk.c */
extern int printk(const char *format, ...);

/* thread.c */
extern ER thread_switch(void);
extern ER thread_release(ID tskid);
extern ER thread_get_id(ID * p_tskid);
extern ER thread_suspend(ID tskid);
extern ER thread_resume(ID tskid);
extern ER thread_change_priority(ID tskid, PRI tskpri);
extern ER rot_rdq(PRI tskpri);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* time.c */
extern void time_initialize(UW seconds);
extern ER time_set(SYSTIM *pk_systim);
extern ER time_get(SYSTIM *pk_systim);
extern ER thread_delay(RELTIM dlytim);
extern void intr_interval(void);
extern void timer_initialize(void);
extern void check_timer(void);

/* rendezvous.c */
extern ER port_initialize(void);
extern ER port_create(ID porid, T_CPOR *pk_cpor);
extern ER_ID port_create_auto(T_CPOR *pk_cpor);
extern ER port_destroy(ID porid);
extern ER_UINT port_call(ID porid, VP msg, UINT cmsgsz);
extern ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg);
extern ER port_reply(RDVNO rdvno, VP msg, UINT rmsgsz);

/* queue.c */
extern ER queue_initialize(void);
extern ER_ID queue_create_auto(T_CDTQ *pk_cdtq);
extern ER queue_destroy(ID dtqid);
extern ER queue_send(ID dtqid, VP_INT data);
extern ER queue_receive(ID dtqid, VP_INT *p_data);

/* api.c */
extern void api_initialize(void);

/* modules.c */
extern void run_init_program(void);

/* palloc.c */
extern void *palloc(size_t size);
extern void pfree(void *addr, size_t size);
extern UW pmemfree(void);

/* kcall.c */
extern void kcall_initialize(void);

/* tree_utils.c */
extern node_t *find_empty_key(tree_t *tree, int *hand);

/* nthread.c */
extern ER thread_initialize(void);
extern T_TCB *get_thread_ptr(ID tskid);
extern ER_ID thread_initialize1(void);
extern ER_ID thread_create_auto(T_CTSK * pk_ctsk);
extern ER thread_destroy(ID tskid);
extern ER thread_start(ID tskid);
extern void thread_end(void);
extern void thread_end_and_destroy(void);
extern ER thread_terminate(ID tskid);

#endif				/* __CORE_FUNC_H__ */
