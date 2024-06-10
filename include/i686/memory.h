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
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE (4096)
#define MPU_MAX_PAGE (1024 * 1024)

#define MPU_LOG_INT (5)

typedef uintptr_t PTE;
typedef uintptr_t PDE;

#define PTE_PER_PAGE (PAGE_SIZE / sizeof(PTE))

#define PAGE_ADDR_MASK 0xfffff000
#define BITS_PAGE (10)
#define MASK_PAGE ((1 << BITS_PAGE) - 1)
#define BITS_OFFSET (12)
#define MASK_OFFSET ((1 << BITS_OFFSET) - 1)
#define BITS_PSE (BITS_PAGE + BITS_OFFSET)
#define MASK_PSE ((1 << BITS_PSE) - 1)

#define SUPERVISOR_START 0x80000000

static inline uintptr_t pageRoundDown(const uintptr_t value)
{
	return value & PAGE_ADDR_MASK;
}

static inline size_t roundUp(const size_t value, const size_t ceil)
{
	return (value + (ceil - 1)) & (~(ceil - 1));
}

static inline uintptr_t pageRoundUp(const uintptr_t value)
{
	return (value + PAGE_SIZE - 1) & PAGE_ADDR_MASK;
}

static inline uintptr_t getDirectoryOffset(const void *addr)
{
	return (uintptr_t) addr >> (BITS_PAGE + BITS_OFFSET);
}

static inline uintptr_t getPageOffset(const void *addr)
{
	return ((uintptr_t) addr >> BITS_OFFSET) & MASK_PAGE;
}

static inline uintptr_t getOffset(const void *addr)
{
	return (uintptr_t) addr & MASK_OFFSET;
}

static inline void *kern_p2v(const void *addr)
{
	return (void *) ((uintptr_t) addr | SUPERVISOR_START);
}

static inline void *kern_v2p(const void *addr)
{
	return (void *) ((uintptr_t) addr & (~SUPERVISOR_START));
}

static inline size_t pages(const size_t bytes)
{
	return (bytes + PAGE_SIZE - 1) >> BITS_OFFSET;
}

static inline void *fault_get_addr(void)
{
	void *address;
	__asm__ __volatile__ (
		"movl %%cr2, %0\n\t"
		:"=a"(address)
		:
		:);
	return address;
}

static inline void paging_set_directory(PDE *address)
{
	__asm__ __volatile__ (
		"movl %0, %%eax\n\t"
		"movl %%eax, %%cr3\n\t"
		:
		:"m"(address)
		:"%eax");
}

static inline void tlb_flush(void *address)
{
	__asm__ __volatile__ (
		"invlpg %0\n\t"
		:
		:"m"(address)
		:);
}

static inline void tlb_flush_all(void)
{
	__asm__ __volatile__ (
		"movl %%cr3, %%eax\n\t"
		"movl %%eax, %%cr3\n\t"
		:
		:
		:"%eax");
}

#endif
