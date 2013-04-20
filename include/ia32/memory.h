#ifndef _IA32_MEMORY_H_
#define _IA32_MEMORY_H_
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
#include <stddef.h>
#include <mpu/config.h>

#define MPU_MAX_PAGE (1024 * 1024)

#define MPU_LOG_INT (5)

typedef UW PTE;

#define PTE_PER_PAGE (PAGE_SIZE / sizeof(PTE))

#define PAGE_ADDR_MASK 0xfffff000
#define BITS_PAGE (10)
#define MASK_PAGE ((1 << BITS_PAGE) - 1)
#define BITS_OFFSET (12)
#define MASK_OFFSET ((1 << BITS_OFFSET) - 1)

static inline void *roundDown(void *addr)
{
	return (void*)((UW)addr & PAGE_ADDR_MASK);
}

static inline void *roundUp(void *addr)
{
	return (void*)(((UW)addr + PAGE_SIZE - 1) & PAGE_ADDR_MASK);
}

static inline UW getDirectoryOffset(const void *addr)
{
	return (UW)addr >> (BITS_PAGE + BITS_OFFSET);
}

static inline UW getPageOffset(const void *addr)
{
	return ((UW)addr >> BITS_OFFSET) & MASK_PAGE;
}

static inline UW getOffset(const void *addr)
{
	return (UW)addr & MASK_OFFSET;
}

static inline void *kern_p2v(const void *addr)
{
	return (void*)((UW)addr | MIN_KERNEL);
}

static inline void *kern_v2p(void *addr)
{
	return (void*)((UW)addr & (~MIN_KERNEL));
}

static inline size_t pages(size_t bytes)
{
	return (bytes + PAGE_SIZE - 1) >> PAGE_SHIFT;
}

#endif
