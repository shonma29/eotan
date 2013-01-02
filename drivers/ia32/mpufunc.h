/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/

#ifndef _IA32_MPUFUNC_H_
#define _IA32_MPUFUNC_H_	1

#include <itron/types.h>
#include "../../kernel/thread.h"

#define halt() asm("hlt")

/* tss.c */
#ifdef TSKSW_DEBUG
extern void print_task_desc(TASK_DESC * desc);
#endif
extern void create_context(T_TCB * task);
extern void init_task_state_segment(W index, T_CONTEXT * tss);

/* virtual_memory.c */
extern ADDR_MAP dup_vmap_table(ADDR_MAP dest);
extern ER release_vmap(ADDR_MAP dest);
extern BOOL vmap(T_TCB * task, UW vpage, UW ppage, W accmode);
extern ER vunmap(T_TCB * task, UW vpage);
extern UW vtor(ID tskid, UW addr);
extern ER region_create(ID id, ID rid, VP start, W min, W max, UW perm, FP handle);
extern ER region_destroy(ID id, ID rid);
extern ER region_map(ID id, VP start, UW size, W accmode);
extern ER region_unmap(ID id, VP start, UW size);
extern ER region_duplicate(ID src, ID dst, ID rid);
extern ER region_get(ID id, VP start, UW size, VP buf);
extern ER region_put(ID id, VP start, UW size, VP buf);
extern ER region_get_status(ID id, ID rid, VP stat);
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

/* fpu.c */
extern void fpu_save(T_TCB * taskp);
extern void fpu_restore(T_TCB * taskp);
extern void fpu_start(T_TCB * taskp);

/* reset_intr_mask.c */
extern void reset_intr_mask(W intn);

/* context.c */
extern ER make_task_context(T_TCB * task, T_CTSK * pk_ctsk);
extern ER mpu_copy_stack(ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi, W edi, ID dst);
extern ER mpu_set_context(ID tid, W eip, B * stackp, W stsize);
extern ER mpu_use_float(ID tid);
extern void set_autorun_context(T_TCB *taskp);
extern void set_thread1_context(T_TCB *taskp);
extern void set_thread1_start(T_TCB *taskp);
extern void set_page_table(T_TCB *taskp, UW p);
extern void set_sp(T_TCB *taskp, UW p);

#endif /* _IA32_MPU_H_ */
