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
#include <mpu/config.h>
#include <mpu/memory.h>
#include <setting.h>
#include "mpufunc.h"
#include "paging.h"

#define ATTR_INITIAL (PAGE_WRITABLE | PAGE_PRESENT)
#ifdef USE_BIG_PAGE
#define ATTR_KERN (PAGE_BIG | PAGE_WRITABLE | PAGE_PRESENT)
#endif

// MIN_KERNEL should be a multiple of 4 MB.
#define OFFSET_KERN (MIN_KERNEL / PAGE_SIZE / PTE_PER_PAGE)

static void set_initial_directories(void);
static UB *set_initial_pages(PTE *p, UB *addr);
static PTE calc_pte(const void *addr, const UW attr);
#ifdef USE_BIG_PAGE
static void set_initial_kern_directories(void);
static PTE calc_kern_pte(const void *addr);
#endif


void paging_initialize(void)
{
	set_initial_directories();
	paging_set_directory((PTE*)PAGE_DIR_ADDR);
	paging_start();
}

static void set_initial_directories(void)
{
	size_t i;
	PTE *dir = (PTE*)PAGE_DIR_ADDR;
	PTE *p = (PTE*)PAGE_ENTRY_ADDR;
	UB *addr = (UB*)0;

	for (i = 0; i < NUM_OF_INITIAL_DIR; i++) {
		dir[i] = calc_pte(p, ATTR_INITIAL);
		dir[OFFSET_KERN + i] =
				calc_pte(MIN_KERNEL + p, ATTR_INITIAL);
		addr = set_initial_pages(p, addr);
		p += PTE_PER_PAGE;
	}

	for (; i < PTE_PER_PAGE / 2; i++) {
		dir[i] = 0;
		dir[OFFSET_KERN + i] = 0;
	}
}

static UB *set_initial_pages(PTE *p, UB *addr)
{
	size_t i;

	for (i = 0; i < PTE_PER_PAGE; i++) {
		p[i] = calc_pte(addr, ATTR_INITIAL);
		addr += PAGE_SIZE;
	}

	return addr;
}

static PTE calc_pte(const void *addr, const UW attr)
{
	return (PTE)((((UW)addr) & PAGE_ADDR_MASK) | attr);
}

#ifdef USE_BIG_PAGE
static void set_initial_kern_directories(void)
{
	size_t i;
	PTE *dir = (PTE*)PAGE_DIR_ADDR;
	UB *addr = (UB*)0;

	for (i = 0; i < NUM_OF_INITIAL_DIR; i++) {
		dir[i] = calc_kern_pte(addr);
		dir[OFFSET_KERN + i] = calc_kern_pte(addr);
		addr += PAGE_SIZE * PTE_PER_PAGE;
	}

	for (; i < PTE_PER_PAGE / 2; i++) {
		dir[i] = 0;
		dir[OFFSET_KERN + i] = 0;
	}
}

static PTE calc_kern_pte(const void *addr)
{
	return (PTE)((((UW)addr) & PAGE_BIG_ADDR_MASK) | ATTR_KERN);
}
#endif
