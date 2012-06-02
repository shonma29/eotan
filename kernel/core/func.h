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
extern W vprintk(B *, VP);

/* error.c */
extern void falldown(B *, ...);

/* memory.c */
extern void init_memory(void);
extern W alloc_memory(W size);
extern ER free_memory(W index, W size);

/* pmemory.c */
extern void pmem_init(void);
extern VP palloc(W size);
extern ER pfree(VP p, W size);
extern void pmemstat(void);
extern UW pmemfree(void);

/* console.c */
extern void simple_init_console(void);
extern BOOL init_crt(W);	/* 初期化                       */
extern W intr_crt(void);	/* 割り込みハンドラ             */

extern void putchar(TC ch);	/* １文字出力                   */
extern void write_cr(void);
extern void write_tab(void);
extern void console_clear(void);
extern ER get_cursor_position(W * x, W * y);

/* timer.c */
extern void start_interval(void);
extern void intr_interval(void);
extern void init_timer(void);
extern void set_timer(W time, void (*func) (VP), VP argp);
extern ER unset_timer(void (*func) (VP), VP arg);
extern void check_timer(void);
extern W left_time(void (*func) (VP), VP arg);

/* virtual_memory.c */
extern ADDR_MAP dup_vmap_table(ADDR_MAP dest);
extern ER release_vmap(ADDR_MAP dest);
extern BOOL vmap(T_TCB * task, UW vpage, UW ppage, W accmode);
extern UW vtor(ID tskid, UW addr);
extern void adjust_vm(UW max_mem);

/* task.c */
extern void init_task(void);
extern void init_task1(void);
extern void init_task_state_segment(W index, T_I386_CONTEXT * tss);
extern ER task_switch(BOOL save_nowtask);
extern void print_task_list(void);
extern T_TCB *get_tskp(ID tskid);
extern void salvage_task(void);
void make_local_stack(T_TCB *tsk, W size, W acc);

/* posix.c */
extern ER posix_kill_proc(ID pid);

/* tss.c */
#ifdef TSKSW_DEBUG
extern void print_task_desc(TASK_DESC * desc);
#endif

/* keyboard.c */
extern W intr_kbd(void);
extern void system_reset(void);

/* kalloc.c */
extern void init_kalloc(void);
extern VP kalloc(W size);
extern void kfree(VP area, W size);

/* message.c */
extern ER init_msgbuf(void);
extern void del_task_mbf(ID tid);

/* eventflag.c */
extern ER init_eventflag(void);
extern void del_task_evt(ID tid);

/* time.c */
extern void init_time(UW seconds);
extern ER set_tim(SYSTIME *pk_tim);
extern ER get_tim(SYSTIME *pk_tim);

/* lowlib.c */
extern ER init_lowlib(struct module_info *modp);
extern ER load_lowlib(VP * argp);
extern ER stat_lowlib(VP * argp);

/* itron_memory.c */
extern ER init_mpl(void);

/* --------------------------------------------------------- */
/* システム管理用タスク                                      */
/* --------------------------------------------------------- */
extern void sys_init_task(void);

/* --------------------------------------------------------- */
/* デバッグ用タスク                                          */
/* --------------------------------------------------------- */
extern void debugger(void);

#ifdef I386
/* tss.c */
extern void create_context(T_TCB * task);
extern void init_task_state_segment(W index, T_I386_CONTEXT * tss);

/* gdt.c */
extern ER set_gdt(int index, GEN_DESC * desc);
extern GEN_DESC *get_gdt(int index);

/* locore.S */
extern void resume(UW);
extern void load_task_register(UW);
extern W get_cr0();
extern W get_cr2();
extern W get_cr3();

/* fpu.h */
void fpu_save(T_TCB * taskp);
void fpu_restore(T_TCB * taskp);
void fpu_start(T_TCB * taskp);
#endif

#endif				/* __CORE_FUNC_H__ */
