#ifndef _MPU_MPUFUNC_H__
#define _MPU_MPUFUNC_H__
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
#include <thread.h>
#include <mm/segment.h>
#include "gate.h"
#include "paging.h"

/* context.c */
extern ER mpu_copy_stack(ID src, W esp, ID dst);
extern ER mpu_set_context(ID tid, W eip, B * stackp, W stsize);
extern ER context_page_fault_handler(void);
extern ER context_mpu_handler(void);

/* ncontext.c */
extern void create_context(thread_t *th);
extern VP_INT *context_create_kernel(VP_INT *sp, const UW eflags, const FP eip);
extern VP_INT *context_create_user(VP_INT *sp, const UW eflags, const FP eip,
		const VP esp);
extern void context_initialize(void);
extern void context_reset_page_cache(const thread_t *task, const VP addr);
extern void context_reset_page_table();
extern void context_switch(thread_t *prev, thread_t *next);

/* virtual_mem.c */
extern ER region_duplicate(ID src, ID dst);
extern ER region_get(ID id, VP start, UW size, VP buf);
extern ER region_map(ID id, VP start, UW size, W accmode);
extern ER region_put(ID id, VP start, UW size, VP buf);
extern ER region_unmap(ID id, VP start, UW size);
extern UW vtor(ID tskid, UW addr);

/* fault.c */
extern void fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW ds, UW no,
		UW eip, UW cs, UW eflags);
extern void fault_with_error(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW ds, UW no,
		UW err, UW eip, UW cs, UW eflags);

/* gate.c */
extern void mpu_initialize(void);
extern void idt_set(UB no, void (*handler)(void));

/* interrupt.c */
extern ER interrupt_initialize(void);
extern void interrupt(const UW edi, const UW esi, const UW ebp, const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW err, const UW eip,
		const UW cs, const W eflags);
ER interrupt_bind(const INHNO inhno, const T_DINH *pk_dinh);

/* paging_init.c */
extern void paging_initialize(void);

/* paging_reset.c */
extern void paging_reset(void);

/* panic.c */
extern void panic(char *msg);

/* util.c */
extern void *getPageAddress(const PTE *dir, const void *addr);
extern ER vmemcpy(const thread_t *th, const void *to, const void *from,
		const size_t bytes);

/* memory.c */
extern PTE *copy_kernel_page_table(const PTE *src);
extern void release_user_pages(PTE *directory);
extern ER copy_user_pages(PTE *dest, const PTE *src, size_t cnt);
extern void release_memory(thread_t *th);

/* handler.s */
extern void service_handler(void);

/* gate_loader.s */
extern void gdt_load(void);
extern void idt_load(void);

/* paging.s */
extern void *fault_get_addr(void);
extern void paging_set_directory(void *dir);
extern void paging_start(void);
extern void tlb_flush_all(void);
extern void tlb_flush(VP addr);

static inline void fpu_save(thread_t **th)
{
	__asm__ __volatile__ ( \
			"fsave %0\n\t" \
			:"=m" ((*th)->mpu.fpu_state));
}

static inline void fpu_restore(thread_t **th)
{
	__asm__ __volatile__ ( \
			"frstor %0\n\t" \
			:"=m" ((*th)->mpu.fpu_state));
}

#endif
