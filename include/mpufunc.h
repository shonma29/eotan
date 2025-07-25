#ifndef _MPUFUNC_H__
#define _MPUFUNC_H__
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
#include <copier.h>
#include <interrupt.h>
#include <stdnoreturn.h>
#include <nerve/thread.h>

/* set_selector.s */
extern noreturn void set_selector(void);

/* context.c */
extern void create_context(thread_t *);
extern void context_release(const thread_t *);
extern void context_switch(thread_t *, thread_t *);

/* gate.c */
extern void mpu_initialize(void);
extern void idt_set(unsigned int no, void (*)(void));

/* interrupt.c */
extern ER interrupt_initialize(void);
extern void interrupt(const UW, const UW, const UW, const UW,
		const UW, const UW, const UW, const UW,
		const UW, const UW, const UW,
		const UW, const W);
extern void interrupt_with_error(const UW, const UW, const UW, const UW,
		const UW, const UW, const UW, const UW,
		const UW, const UW, const UW, const UW,
		const UW, const W);
ER_ID interrupt_bind(T_CISR *);
ER interrupt_unbind(ID);

/* define_mpu_handlers */
int define_mpu_handlers(void (*)(VP_INT), void (*)(VP_INT));

/* paging_init.c */
extern void paging_initialize(const size_t);

/* paging_reset.c */
extern void paging_reset(void);

/* panic.c */
extern noreturn void panic(char *);

/* address.c */
extern void *getPageAddress(const PTE *, const void *);

/* get_writable_page.c */
extern void *get_writable_page(const PTE *, const void *);

/* memcpy.c */
extern ER memcpy_k2u(thread_t *, void *, const void *, const size_t);
extern ER memcpy_u2k(thread_t *, void *, const void *,
		const size_t);

/* strncpy.c */
extern ER_UINT strncpy_u2k(thread_t *, void *, const void *,
		const size_t);

/* scattered_copy.c */
extern ER scattered_copy_u2k(const thread_t *, const size_t, copy_range_t *,
		const size_t);

/* memory.c */
extern PTE *copy_kernel_page_table(void);
extern int copy_user_pages(void *, const void *, const void *, const size_t,
		const int);
extern int attach_user_pages(void *, const void *, const void *, const size_t,
		const int);
extern ER map_user_pages(PTE *, VP, size_t, const int);
extern int unmap_user_pages(void *, const void *, const size_t);
extern int detach_user_pages(void *, const void *, const size_t);
extern ER move_stack(const PTE *, void *, const void *,
		const size_t);

/* gate_loader.s */
extern void gdt_load(void);
extern void idt_load(void);

/* paging_start.s */
extern void paging_start(void);

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

/* fiber.s */
extern bool fiber_start(void *, void *, void *, void **, void **);
extern bool fiber_switch(void **, void **);

#endif
