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
#include <mpu/memory.h>
#include "../../kernel/thread.h"
#include "gate.h"

/* tss.c */
#ifdef TSKSW_DEBUG
extern void print_task_desc(TASK_DESC * desc);
#endif
extern void create_context(T_TCB * task);

/* virtual_memory.c */
extern ADDR_MAP dup_vmap_table(ADDR_MAP dest);
extern ER release_vmap(ADDR_MAP dest);
extern BOOL vmap(T_TCB * task, UW vpage, UW ppage, W accmode);
extern ER vunmap(T_TCB * task, UW vpage);
extern UW vtor(ID tskid, UW addr);
extern ER region_create(ID id, ID rid, VP start, W min, W max, UW perm);
extern ER region_destroy(ID id, ID rid);
extern ER region_map(ID id, VP start, UW size, W accmode);
extern ER region_unmap(ID id, VP start, UW size);
extern ER region_duplicate(ID src, ID dst);
extern ER region_get(ID id, VP start, UW size, VP buf);
extern ER region_put(ID id, VP start, UW size, VP buf);
extern ER region_get_status(ID id, ID rid, VP stat);

/* interrupt.c */
extern volatile W	on_interrupt;
extern BOOL	delayed_dispatch;

extern W	init_interrupt (void);
extern void	interrupt (W);
extern void	page_fault (UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW err, UW eip, UW cs, W eflags);
extern void	set_idt (UW, UW, UW, UW, UW);
extern void	protect_fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		   UW ecx, UW eax, UW es, UW ds, UW no,
		   UW err, UW eip, UW cs, UW eflags);
extern ER  	set_interrupt_entry (W intno, FP func, ATR attr);
extern W	int32_handler (void);
extern W	int33_handler (void);
extern W	int38_handler (void);	/* floppy disk */
extern W        int44_handler (void);   /* psaux */
extern W	int46_handler (void);

/* locore.S */
extern void resume(UW);
extern void load_task_register(UW);

/* fpu.c */
extern void fpu_initialize(void);
extern void fpu_save(T_TCB * taskp);
extern void fpu_restore(T_TCB * taskp);
extern void fpu_start(T_TCB * taskp);

/* context.c */
extern ER make_task_context(T_TCB * task, T_CTSK * pk_ctsk);
extern ER mpu_copy_stack(ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi, W edi, ID dst);
extern ER mpu_set_context(ID tid, W eip, B * stackp, W stsize);
extern void set_thread1_context(T_TCB *taskp);
extern void set_thread1_start(T_TCB *taskp);
extern void set_page_table(T_TCB *taskp, UW p);
extern void set_sp(T_TCB *taskp, UW p);

/* gate.c */
extern void idt_initialize(void);
extern void idt_set(UB no, UH selector, W (*handler)(void),
		UB type, UB dpl);
extern void gate_set(GateDescriptor *p,
		UH selector, W (*handler)(void), UB attr);
extern void gdt_initialize(void);

/* fault.c */
extern void fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW eip, UW cs, UW eflags);
extern void fault_with_error(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW err, UW eip, UW cs, UW eflags);

/* gate_loader.s */
extern void gdt_load(void);
extern void idt_load(void);

/* abort.s */
extern W handle0(void);
extern W handle1(void);
extern W handle2(void);
extern W handle3(void);
extern W handle4(void);
extern W handle5(void);
extern W handle6(void);
extern W handle7(void);
extern W handle8(void);
extern W handle9(void);
extern W handle10(void);
extern W handle11(void);
extern W handle12(void);
extern W handle13(void);
extern W handle14(void);
extern W handle15(void);
extern W handle16(void);
extern W handle17(void);
extern W handle18(void);
extern W handle19(void);
extern W handle20(void);
extern W handle21(void);
extern W handle22(void);
extern W handle23(void);
extern W handle24(void);
extern W handle25(void);
extern W handle26(void);
extern W handle27(void);
extern W handle28(void);
extern W handle29(void);
extern W handle30(void);
extern W handle31(void);
extern W service_handler(void);

/* paging.s */
extern void *fault_get_addr(void);
extern void paging_set_directory(void *dir);
extern void paging_start(void);
extern void tlb_flush(void);

/* paging_init.c */
extern void paging_initialize(void);

/* paging_reset.c */
void paging_reset(void);

/* util.c */
extern ER vmemcpy(const T_TCB *th, const void *to, const void *from,
		const size_t bytes);
extern void *getPageAddress(const PTE *dir, const void *addr);

/* panic.c */
extern void panic(char *msg);

#endif /* _IA32_MPU_H_ */
