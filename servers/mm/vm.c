#if 0
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
#include <string.h>
#include <core.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include <func.h>
#include <thread.h>
#include "../../kernel/mpu/mpufunc.h"

static BOOL vmap(thread_t *th, UW vpage, UW ppage, W accmode);
static ER vunmap(thread_t *th, UW vpage);


ER mm_process_create(ID id, ID rid, VP start, W min,  W max, UW perm)
{
	thread_t *th;
	T_REGION *regp;

	if (min < 0)
		return E_PAR;
	if (max <= 0)
		return E_PAR;
	if (min > max)
		return E_PAR;

	th = get_thread_ptr(id);
	if (!th)
		return E_OBJ;

	if (rid < 0 || rid >= MAX_REGION)
		return E_PAR;
	if (th->regions[rid].permission)
		return E_OBJ;

	regp = &(th->regions[rid]);
	regp->start_addr = (VP)pageRoundDown((UW)start);
	regp->min_size = pageRoundUp(min);
	regp->max_size = pageRoundUp(max);
	regp->permission = perm;

	return E_OK;
}

ER mm_process_destroy(ID id)
{
	ID rid;
	thread_t *th = get_thread_ptr(id);

	if (!th)
		return E_OBJ;

	for (rid = TEXT_REGION; rid <= HEAP_REGION; rid++) {
		mm_process_vunmap(id, th->regions[rid].start_addr,
				th->regions[rid].min_size);
		th->regions[rid].permission = 0;
	}

	return E_OK;
}

ER mm_process_duplicate(ID src_id, ID dest_id)
{
	thread_t *dest;
	thread_t *src = (thread_t*)get_thread_ptr(src_id);

	if (!src)
		return E_PAR;

	dest = (thread_t*)get_thread_ptr(dest_id);
	if (!dest)
		return E_PAR;

	dest->regions[TEXT_REGION] = src->regions[TEXT_REGION];
	dest->regions[DATA_REGION] = src->regions[DATA_REGION];
	dest->regions[HEAP_REGION] = src->regions[HEAP_REGION];

	return copy_user_pages(MPU_PAGE_TABLE(dest), MPU_PAGE_TABLE(src),
			(pageRoundUp(dest->regions[TEXT_REGION].min_size) >> BITS_OFFSET)
			+ (pageRoundUp(dest->regions[DATA_REGION].min_size) >> BITS_OFFSET)
			+ (pageRoundUp(dest->regions[HEAP_REGION].min_size) >> BITS_OFFSET)
	);
}

static BOOL vmap(thread_t *th, UW vpage, UW ppage, W accmode)
{
	PTE *directory;
	PTE *pages;
	UINT index;

	if (vpage >= MIN_KERNEL)
		return TRUE;

	directory = (PTE*)kern_p2v(MPU_PAGE_TABLE(th));
	index = (vpage & DIR_MASK) >> DIR_SHIFT;

	if (!is_present(directory[index])) {
		pages = (PTE*)palloc();
		if (!pages)
			return FALSE;

		directory[index] = calc_pte(kern_v2p(pages),
				(accmode & ACC_USER)? ATTR_USER:ATTR_INITIAL);
	} else
		pages = (PTE*)(directory[index] & PAGE_ADDR_MASK);

	if ((UW)pages <= KERNEL_SIZE)
		pages = (PTE*)(kern_p2v(pages));

	index = (vpage & PAGE_MASK) >> PAGE_SHIFT;
	if (is_present(pages[index]))
		printk("vmap: vpage %x has already mapped\n", vpage);

	pages[index] = calc_pte(kern_v2p((void*)ppage),
			(accmode & ACC_USER)? ATTR_USER:ATTR_INITIAL);

	context_reset_page_cache(th, (VP)vpage);

	return TRUE;
}

ER mm_vmap(ID id, VP start, UW size, W accmode)
{
	UW i;
	ER result;
	T_REGION *regp;
	thread_t *th = (thread_t*)get_thread_ptr(id);

	if (!th)
		return E_PAR;

	size = pages(size);
	if (pmemfree() < size)
		return E_NOMEM;

	regp = find_region(th, start);
	start = (VP)pageRoundDown((UW)start);
	result = E_OK;

	for (i = 0; i < size; i++) {
		VP pmem = palloc();

		if (!pmem) {
			result = E_NOMEM;
			break;
		}

		if (!vmap(th, ((UW)start + (i << PAGE_SHIFT)),
				(UW)kern_v2p(pmem), accmode)) {
			pfree((VP)kern_v2p(pmem));
			result = E_SYS;
			break;
		}
	}

	if (result != E_OK) {
		UW j;

		for (j = 0; j < i; ++j)
			vunmap(th, (UW)start + (j << PAGE_SHIFT));

	} else if (regp) {
		if (((UW)start) + (size << PAGE_SHIFT) > (UW)regp->start_addr) {
			UW newsize = (UW)start + (size << PAGE_SHIFT)
					- (UW)regp->start_addr;

			if (newsize > regp->min_size) {
				regp->min_size = newsize;
				if (regp->min_size > regp->max_size)
					printk("[WARNING] region_map: min_size exceeds max_size\n");
			}
		}
	}

	return result;
}

static ER vunmap(thread_t *th, UW vpage)
{
	PTE *directory;
	PTE *pages;
	UINT index;

	directory = (PTE*)kern_p2v(MPU_PAGE_TABLE(th));
	index = (vpage & DIR_MASK) >> DIR_SHIFT;

	if (!is_present(directory[index]))
		return FALSE;
	else
		pages = (PTE*)(directory[index] & PAGE_ADDR_MASK);

	if ((UW)pages <= KERNEL_SIZE)
		pages = (PTE*)(kern_p2v(pages));

	index = (vpage & PAGE_MASK) >> PAGE_SHIFT;
	/*TODO handle error */
	pfree(kern_v2p((void*)(pages[index] & PAGE_ADDR_MASK)));
	pages[index] = 0;

	context_reset_page_cache(th, (VP)vpage);

	return TRUE;
}

ER mm_vunmap(ID id, VP start, UW size)
{
	UW i;
	T_REGION *regp;
	thread_t *th = (thread_t*)get_thread_ptr(id);

	if (!th)
		return E_PAR;

	regp = find_region(th, start);
	size = pageRoundUp(size);
	start = (VP)pageRoundDown((UW)start);

	for (i = 0; i < (size >> PAGE_SHIFT); i++)
		if (!vunmap(th, ((UW)start + (i << PAGE_SHIFT))))
			return E_SYS;

	if (regp)
		if (start >= regp->start_addr) {
			UW newsize = (UW)(start - regp->start_addr);

			if (newsize + size == regp->min_size)
				regp->min_size = newsize;
		}

	return E_OK;
}

ER mm_vmstatus(ID id, ID rid, VP stat)
{
	T_REGION *p = stat;
	thread_t *th = get_thread_ptr(id);

	if (!th)
		return E_NOEXS;

	if (rid < 0 || rid >= MAX_REGION)
		return E_PAR;
	if (!th->regions[rid].permission)
		return E_OBJ;

	*p = th->regions[rid];

	return E_OK;
}
#endif
