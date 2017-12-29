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
extern void create_context(thread_t *th);
extern void context_initialize(void);
extern void context_reset_page_cache(const VP page_table, const VP addr);
extern void context_reset_page_table();
extern void context_switch(thread_t *prev, thread_t *next);

/* gate.c */
extern void mpu_initialize(void);
extern void idt_set(UB no, void (*handler)(void));

/* interrupt.c */
extern ER interrupt_initialize(void);
extern FP interrupt(const UW edi, const UW esi, const UW ebp, const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW eip,
		const UW cs, const W eflags);
extern FP interrupt_with_error(const UW edi, const UW esi, const UW ebp, const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW err, const UW eip,
		const UW cs, const W eflags);
ER interrupt_bind(const INHNO inhno, const T_DINH *pk_dinh);

/* define_mpu_handlers */
void define_mpu_handlers(const FP, const FP);

/* paging_init.c */
extern void paging_initialize(void);

/* paging_reset.c */
extern void paging_reset(void);

/* panic.c */
extern void panic(char *msg);

/* address.c */
extern void *getPageAddress(const PTE *dir, const void *addr);

/* util.c */
extern ER memcpy_k2u(thread_t *th, void *to, const void *from, const size_t bytes);
extern ER memcpy_u2k(thread_t *th, void *to, const void *from,
		const size_t bytes);
extern ER_UINT strncpy_u2k(thread_t *th, void *to, const void *from,
		const size_t bytes);

/* memory.c */
extern PTE *copy_kernel_page_table(void);
extern ER copy_user_pages(PTE *dest, const PTE *src, size_t cnt);
extern ER map_user_pages(PTE *dir, VP addr, size_t cnt);
extern ER unmap_user_pages(PTE *dir, VP addr, size_t cnt);
extern ER move_stack(const PTE *page_table, void *to, const void *from,
		const size_t bytes);

/* gate_loader.s */
extern void gdt_load(void);
extern void idt_load(void);

/* paging.s */
extern void *fault_get_addr(void);
extern void paging_set_directory(void *dir);
extern void paging_start(void);
extern void tlb_flush_all(void);
extern void tlb_flush(VP addr);

/* service_handler.s */
extern void service_handler(void);

/* service_initialize */
extern void service_initialize(void);

static inline void fpu_save(thread_t **th)
{
	__asm__ __volatile__ ( \
			"fxsave %0\n\t" \
			:"=m" ((*th)->mpu.fpu_state));
}

static inline void fpu_restore(thread_t **th)
{
	__asm__ __volatile__ ( \
			"fxrstor %0\n\t" \
			:"=m" ((*th)->mpu.fpu_state));
}

#endif
