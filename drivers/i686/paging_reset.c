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
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/memory_map.h>
#include <nerve/func.h>
#include "paging.h"


//TODO unify paging_init
//TODO skip memory hole
void paging_reset(void)
{
	PDE *dir = (PDE *) kern_p2v((void *) KTHREAD_DIR_ADDR);
	uintptr_t addr = 0;
	MemoryMap *mm = &(sysinfo->memory_map);
	size_t max = (mm->max_pages + PTE_PER_PAGE - 1) / PTE_PER_PAGE;
	size_t left = mm->max_pages;
	printk("reset max=%x left=%x\n", max, left);

	for (unsigned int i = 0; i < max; i++) {
		PTE *p = palloc();
		if (!p) {
			printk("no memory for PTE");
			break;
		}

		unsigned int j;
		for (j = 0; left && (j < PTE_PER_PAGE); j++) {
			p[j] = calc_pte((void *) addr, ATTR_INITIAL);
			addr += PAGE_SIZE;
			left--;
		}

		for (; j < PTE_PER_PAGE; j++) {
			p[j] = 0;
			addr += PAGE_SIZE;
		}

		dir[OFFSET_KERN + i] = calc_pte(kern_v2p(p), ATTR_INITIAL);
	}

	tlb_flush_all();
}
