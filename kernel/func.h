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


#include <itron/rendezvous.h>
#include <mpu/memory.h>
#include "core.h"
#include "boot.h"
#include "thread.h"

/* main.c */
extern ER main(void);
extern ER add_trmtbl(ID, ID, ID);

/* printk.c */
extern int printk(const char *format, ...);

/* error.c */
extern void falldown();

/* thread.c */
extern void thread_initialize(void);
extern void thread_initialize1(void);
extern ER thread_switch(void);
extern ER thread_create(ID tskid, T_CTSK * pk_ctsk);
extern ER thread_start(ID tskid, INT stacd);
extern ER thread_destroy(ID tskid);
extern void thread_end(void);
extern void thread_end_and_destroy(void);
extern ER thread_terminate(ID tskid);
extern ER thread_release(ID tskid);
extern ER thread_get_id(ID * p_tskid);
extern ER thread_suspend(ID taskid);
extern ER thread_resume(ID taskid);
extern ER thread_change_priority(ID tskid, PRI tskpri);
extern ER rot_rdq(PRI tskpri);
extern ER_ID thread_create_auto(T_CTSK * pk_ctsk);
extern T_TCB *get_thread_ptr(ID tskid);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* message.c */
extern ER queue_initialize(void);
extern ER queue_create(ID id, T_CMBF * pk_cmbf);
extern ER_ID queue_create_auto(T_CMBF * pk_cmbf);
extern ER queue_destroy(ID id);
extern ER message_send(ID id, INT size, VP msg);
extern ER message_send_nowait(ID id, INT size, VP msg);
extern ER message_receive(VP msg, INT * size, ID id);

/* flag.c */
extern ER flag_initialize(void);
extern ER_ID flag_create_auto(T_CFLG * pk_flg);
extern ER flag_destroy(ID flgid);
extern ER flag_set(ID flgid, UINT setptn);
extern ER flag_clear(ID flgid, UINT clrptn);
extern ER flag_wait(UINT * flgptn, ID flgid, UINT waiptn, UINT wfmode);

/* time.c */
extern void time_initialize(UW seconds);
extern ER time_set(SYSTIME *pk_tim);
extern ER time_get(SYSTIME *pk_tim);
extern ER thread_delay(DLYTIME dlytim);
extern ER alarm_create(HNO almno, T_DALM *pk_dalm);
extern void intr_interval(void);
extern void timer_initialize(void);
extern void check_timer(void);

/* rendezvous.c */
ER port_initialize(void);
ER port_create(ID porid, T_CPOR *pk_cpor);
ER_ID port_create_auto(T_CPOR *pk_cpor);
ER port_destroy(ID porid);
ER_UINT port_call(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz);
ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg);
ER port_reply(RDVNO rdvno, VP msg, UINT rmsgsz);

/* api.c */
extern void api_initialize(void);

/* modules.c */
extern void run_init_program(void);

/* palloc.c */
extern void *palloc(size_t size);
extern void pfree(void *addr, size_t size);
extern UW pmemfree(void);

#endif				/* __CORE_FUNC_H__ */
