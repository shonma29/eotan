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


#include "core.h"
#include "boot.h"
#include "thread.h"
#include "../include/itron/rendezvous.h"

#include "../servers/port-manager/port-manager.h"

/* main.c */
extern ER main(void);
extern ER add_trmtbl(ID, ID, ID);

/* printk.c */
extern W printk(B *, ...);

/* error.c */
extern void falldown(B *, ...);

/* pmemory.c */
extern void init_memory(void);
extern void pmem_init(void);
extern VP palloc(W size);
extern ER pfree(VP p, W size);
extern void pmemstat(void);
extern UW pmemfree(void);

/* thread.c */
extern void thread_initialize(void);
extern void thread_initialize1(void);
extern ER thread_switch(void);
extern void print_thread_list(void);
extern T_TCB *get_thread_ptr(ID tskid);
extern void make_local_stack(T_TCB *tsk, W size, W acc);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* kalloc.c */
extern void init_kalloc(void);
extern VP kalloc(W size);
extern void kfree(VP area, W size);

/* message.c */
extern ER queue_initialize(void);

/* flag.c */
extern ER flag_initialize(void);

/* time.c */
extern void time_initialize(UW seconds);
extern ER set_tim(SYSTIME *pk_tim);
extern ER get_tim(SYSTIME *pk_tim);

/* lowlib.c */
extern ER init_lowlib(struct module_info *modp);
extern ER load_lowlib(VP * argp);
extern ER stat_lowlib(VP * argp);


/* rendezvous.c */
ER port_initialize(void);
ER port_create(ID porid, T_CPOR *pk_cpor);
ER_ID port_create_auto(T_CPOR *pk_cpor);
ER port_destroy(ID porid);
ER_UINT port_call(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz);
ER_UINT port_accept(ID porid, RDVNO *p_rdvno, VP msg);
ER port_reply(RDVNO rdvno, VP msg, UINT rmsgsz);


#endif				/* __CORE_FUNC_H__ */
