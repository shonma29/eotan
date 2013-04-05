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
#include <stddef.h>
#include <mpu/memory.h>
#include <mpu/mpufunc.h>
#include <func.h>
#include <memory_map.h>
#include <setting.h>
#include "paging.h"

static void extend(void);


void paging_clean(void)
{
	size_t i;
	PTE *dir = (PTE*)(PAGE_DIR_ADDR | MIN_KERNEL);

	/* release low memory from page directory */
	for (i = 0; i < NUM_OF_INITIAL_DIR; i++)
		dir[i] = 0;

	// set high memory to page directory and PTE
	extend();

	tlb_flush();
}

static void extend(void)
{
	MemoryMap *mm = (MemoryMap*)MEMORY_MAP_ADDR;
	PTE *dir = (PTE*)(PAGE_DIR_ADDR | MIN_KERNEL);
	UB *addr = (UB*)MIN_MEMORY_SIZE;
	size_t i;
	size_t max = (mm->max_pages + PTE_PER_PAGE - 1) / PTE_PER_PAGE;
	size_t left = mm->max_pages - NUM_OF_INITIAL_DIR * PTE_PER_PAGE;

	printk("[KERN] extend addr=%p max=%x left=%x\n", addr, max, left);

	for (i = NUM_OF_INITIAL_DIR; i < max; i++) {
		size_t j;
		PTE *p = palloc(1);

		if (!p) {
			printk("[KERN] no memory for PTE");
			break;
		}

		for (j = 0; left && (j < PTE_PER_PAGE); j++) {
			p[j] = calc_pte(addr, ATTR_INITIAL);
			addr += PAGE_SIZE;
			left--;
		}

		for (; j < PTE_PER_PAGE; j++) {
			p[j] = 0;
			addr += PAGE_SIZE;
		}

		dir[OFFSET_KERN + i] = calc_pte(p, ATTR_INITIAL);
	}
}
