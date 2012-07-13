/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/

#ifndef _IA32_MPUFUNC_H_
#define _IA32_MPUFUNC_H_	1

#include "../../include/itron/types.h"
#include "../../kernel/task.h"

/* tss.c */
#ifdef TSKSW_DEBUG
extern void print_task_desc(TASK_DESC * desc);
#endif
extern void create_context(T_TCB * task);
extern void init_task_state_segment(W index, T_I386_CONTEXT * tss);

/* virtual_memory.c */
extern ADDR_MAP dup_vmap_table(ADDR_MAP dest);
extern ER release_vmap(ADDR_MAP dest);
extern BOOL vmap(T_TCB * task, UW vpage, UW ppage, W accmode);
extern UW vtor(ID tskid, UW addr);
extern void adjust_vm(UW max_mem);

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

#endif /* _IA32_MPU_H_ */
