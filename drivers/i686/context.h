#ifndef _MPU_CONTEXT_H_
#define _MPU_CONTEXT_H_
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
#include <mpu/memory.h>

typedef struct
{
	void *esp0;
	PTE *cr3;
	int pad[2];
	char fpu_state[512];
} mpu_context_t;

static inline void context_set_kernel_sp(mpu_context_t *ctx, const void *addr)
{
	ctx->esp0 = (void*)addr;
}

static inline void context_set_page_table(mpu_context_t *ctx, const void *addr)
{
	ctx->cr3 = (PTE*)addr;
}

static inline void tr_set(const unsigned short selector)
{
	__asm__ __volatile__ ( \
			"ltr %w0\n\t" \
			: \
			:"r"(selector));
}

static inline void stack_switch_wrapper(void *prev, void *next)
{
	__asm__ __volatile__ ( \
			"movl %0, %%ecx\n\t" \
			"movl %1, %%edx\n\t" \
			"call stack_switch\n\t" \
			: \
			:"g"(prev), "g"(next)
			:"%eax","%ecx","%edx");
}

/* switch.s */
extern void stack_switch(void);

#endif
