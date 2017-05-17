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
#include <nerve/config.h>
#include <nerve/memory_map.h>
#include <func.h>
#include "paging.h"

#ifdef USE_VESA
#include <vesa.h>

static void set_frame_buffer(PTE *dir);
#endif


void paging_reset(void)
{
	MemoryMap *mm = (MemoryMap*)MEMORY_MAP_ADDR;
	PTE *dir = (PTE*)kern_p2v((void*)KTHREAD_DIR_ADDR);
	UB *addr = (UB*)0;
	size_t i;
	size_t max = (mm->max_pages + PTE_PER_PAGE - 1) / PTE_PER_PAGE;
	size_t left = mm->max_pages;

	printk("reset addr=%p max=%x left=%x\n", addr, max, left);

	for (i = 0; i < max; i++) {
		size_t j;
		PTE *p = palloc();

		if (!p) {
			printk("no memory for PTE");
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

		dir[OFFSET_KERN + i] = calc_pte(kern_v2p(p), ATTR_INITIAL);
	}

	tlb_flush_all();
#ifdef USE_VESA
	set_frame_buffer(dir);
#endif
}

#ifdef USE_VESA
static void set_frame_buffer(PTE *dir)
{
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);
	UW start = v->buffer_addr >> BITS_OFFSET;
	UW last = v->buffer_addr + v->bytes_per_line * v->height;
	size_t i;

	last = (last >> BITS_OFFSET) + ((last & MASK_OFFSET)? 1:0);
	printk("VESA start=%x last=%x\n", start, last);

	for (i = start >> BITS_PAGE;
			i < (last >> BITS_PAGE) + ((last & MASK_PAGE)? 1:0);
			i++) {
		if (!(dir[i] & PAGE_PRESENT)) {
			UB *p = (UB*)palloc();

			/* memset(p, 0, PAGE_SIZE); */
			dir[i] = calc_pte(kern_v2p(p), ATTR_INITIAL);
		}
	}

	for (i = start; i < last; i++) {
		size_t offset_dir = i >> BITS_PAGE;
		size_t offset_page = i & MASK_PAGE;
		PTE *entry = (PTE*)kern_p2v((void*)(dir[offset_dir]
				& (~MASK_OFFSET)));
/*
		if (entry[offset_page] & PAGE_PRESENT) {
			//TODO release page
		}
*/
		entry[offset_page] = calc_pte((void*)(i << BITS_OFFSET),
				ATTR_INITIAL);
	}

	tlb_flush_all();
}
#endif
