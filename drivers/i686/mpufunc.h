/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/

#ifndef _MPU_MPUFUNC_H_
#define _MPU_MPUFUNC_H_	1

#include <core.h>
#include <mpu/memory.h>
#include "../../kernel/thread.h"
#include "gate.h"

/* virtual_memory.c */
extern ADDR_MAP dup_vmap_table(ADDR_MAP dest);
extern ER release_vmap(ADDR_MAP dest);
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
extern void	protect_fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		   UW ecx, UW eax, UW es, UW ds, UW no,
		   UW err, UW eip, UW cs, UW eflags);
extern ER  	interrupt_bind (W inhno, T_DINH *pk_dinh);
extern W	int32_handler (void);
extern W	int33_handler (void);

/* fpu.c */
extern void fpu_initialize(void);
extern void fpu_save(thread_t * taskp);
extern void fpu_restore(thread_t * taskp);
extern void fpu_start(thread_t * taskp);

/* context.c */
extern ER create_context(thread_t * task, T_CTSK * pk_ctsk);
extern ER mpu_copy_stack(ID src, W esp, ID dst);
extern ER mpu_set_context(ID tid, W eip, B * stackp, W stsize);
extern void set_page_table(thread_t *taskp, VP p);
extern void set_arg(thread_t *taskp, const UW arg);

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
extern void tlb_flush_all(void);
extern void tlb_flush(VP addr);

/* paging_init.c */
extern void paging_initialize(void);

/* paging_reset.c */
void paging_reset(void);

/* util.c */
extern ER vmemcpy(const thread_t *th, const void *to, const void *from,
		const size_t bytes);
extern void *getPageAddress(const PTE *dir, const void *addr);

/* panic.c */
extern void panic(char *msg);

/* switch.s */
extern void tr_set(const UW selector);
extern void context_switch(void);

/* ncontext.c */
extern VP *context_prev_sp;
extern VP *context_next_sp;
extern void context_initialize(void);
extern void context_set_kernel_sp(const VP addr);
extern VP_INT *context_create_kernel(VP_INT *sp, const UW eflags, const FP eip);
extern VP_INT *context_create_user(VP_INT *sp, const UW eflags, const FP eip,
		const VP esp);
extern void context_switch_page_table(thread_t *next);
extern void context_reset_page_table();
extern void context_reset_page_cache(const thread_t *task, const VP addr);

#endif /* _MPU_MPU_H_ */
