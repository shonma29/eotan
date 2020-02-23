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

static void set_initial_directories(void);
static PTE calc_kern_pte(const void *);


void paging_initialize(void)
{
	set_initial_directories();
	paging_set_directory((PTE *) KTHREAD_DIR_ADDR);
	paging_start();
}

static void set_initial_directories(void)
{
	size_t i;
	PTE *dir = (PTE *) KTHREAD_DIR_ADDR;
	UB *addr = (UB *) 0;

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
	return (PTE) ((((uintptr_t) addr) & PAGE_BIG_ADDR_MASK) | ATTR_BIG);
}
