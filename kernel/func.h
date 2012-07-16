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
#include "task.h"

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

/* task.c */
extern void init_task(void);
extern void init_task1(void);
extern ER task_switch(BOOL save_nowtask);
extern void print_task_list(void);
extern T_TCB *get_tskp(ID tskid);
extern void make_local_stack(T_TCB *tsk, W size, W acc);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* kalloc.c */
extern void init_kalloc(void);
extern VP kalloc(W size);
extern void kfree(VP area, W size);

/* message.c */
extern ER init_msgbuf(void);

/* eventflag.c */
extern ER init_eventflag(void);

/* time.c */
extern void init_time(UW seconds);
extern ER set_tim(SYSTIME *pk_tim);
extern ER get_tim(SYSTIME *pk_tim);

/* lowlib.c */
extern ER init_lowlib(struct module_info *modp);
extern ER load_lowlib(VP * argp);
extern ER stat_lowlib(VP * argp);

#endif				/* __CORE_FUNC_H__ */
