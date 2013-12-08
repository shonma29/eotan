#ifndef _MPU_MEMORY_H_
#define _MPU_MEMORY_H_
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
#include <core/types.h>
#include <stddef.h>

#define PAGE_SIZE (4096)
#define MPU_MAX_PAGE (1024 * 1024)

#define MPU_LOG_INT (5)

typedef UW PTE;

#define PTE_PER_PAGE (PAGE_SIZE / sizeof(PTE))

#define PAGE_ADDR_MASK 0xfffff000
#define BITS_PAGE (10)
#define MASK_PAGE ((1 << BITS_PAGE) - 1)
#define BITS_OFFSET (12)
#define MASK_OFFSET ((1 << BITS_OFFSET) - 1)

#define SUPERVISOR_START 0x80000000

static inline UW pageRoundDown(const UW value)
{
	return value & PAGE_ADDR_MASK;
}

static inline UW roundUp(const UW value, const UW ceil)
{
	return (value + (ceil - 1)) & (~(ceil - 1));
}

static inline UW pageRoundUp(const UW value)
{
	return (value + PAGE_SIZE - 1) & PAGE_ADDR_MASK;
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
	return (void*)((UW)addr | SUPERVISOR_START);
}

static inline void *kern_v2p(const void *addr)
{
	return (void*)((UW)addr & (~SUPERVISOR_START));
}

static inline size_t pages(const size_t bytes)
{
	return (bytes + PAGE_SIZE - 1) >> BITS_OFFSET;
}

#endif
