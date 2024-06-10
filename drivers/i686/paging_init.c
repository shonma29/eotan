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
#include <stdint.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include "mpufunc.h"
#include "paging.h"

static void set_initial_pages(const size_t);
static PTE calc_kern_pte(const void *);


void paging_initialize(const size_t num_of_pages)
{
	set_initial_pages(num_of_pages);
	paging_set_directory((PDE *) KTHREAD_DIR_ADDR);
	paging_start();
}

static void set_initial_pages(const size_t num_of_pages)
{
	size_t pse_pages = (num_of_pages + MASK_PAGE) >> BITS_PAGE;
	PDE *dir = (PDE *) KTHREAD_DIR_ADDR;
	uintptr_t addr = 0;
	unsigned int i;
	for (i = 0; i < pse_pages; i++) {
		dir[OFFSET_KERN + i] = dir[i] = calc_kern_pte((void *) addr);
		addr += PAGE_SIZE * PTE_PER_PAGE;
	}

	for (; i < PTE_PER_PAGE / 2; i++)
		dir[OFFSET_KERN + i] = dir[i] = 0;
}

static PTE calc_kern_pte(const void *addr)
{
	return (PTE) ((((uintptr_t) addr) & PAGE_BIG_ADDR_MASK) | ATTR_BIG);
}
