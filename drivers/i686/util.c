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
#include <string.h>
#include <mpu/memory.h>
#include <thread.h>
#include "paging.h"
#include "mpufunc.h"
#include "func.h"

static inline PTE *getPageDirectory(const thread_t *th)
{
	return (PTE*)(th->mpu.context.cr3);
}


ER vmemcpy(const thread_t *th, const void *to, const void *from,
		const size_t bytes)
{
	PTE *dir = (PTE*)kern_p2v(getPageDirectory(th));
	UB *w = (UB*)to;
	UB *r = (UB*)from;
	size_t left = bytes;
	size_t offset = getOffset(w);

	if (offset) {
		void *p = getPageAddress(dir, w);
		size_t len;
	
		if (!p)
			return E_PAR;

		len = PAGE_SIZE - offset;
		if (len > left)
			len = left;

		memcpy(p + offset, r, len);
		w += len;
		r += len;
		left -= len;
	}

	while (left) {
		void *p = getPageAddress(dir, w);
		size_t len;
	
		if (!p)
			return E_PAR;

		len = PAGE_SIZE;
		if (len > left)
			len = left;

		memcpy(p, r, len);
		w += len;
		r += len;
		left -= len;
	}

	return E_OK;
}

void *getPageAddress(const PTE *dir, const void *addr)
{
	PTE pte = dir[getDirectoryOffset(addr)];

	if (pte & PAGE_PRESENT) {
		PTE *table = (PTE*)kern_p2v((void*)(pte & PAGE_ADDR_MASK));

		pte = table[getPageOffset(addr)];
		if (pte & PAGE_PRESENT)
			return kern_p2v((void*)(pte & PAGE_ADDR_MASK));
	}

	return NULL;
}

T_REGION *find_region(const thread_t *th, const void *addr)
{
	size_t i;

	for (i = 0; i < MAX_REGION; i++) {
		T_REGION *r = (T_REGION*)&(th->regions[i]);

		if (!(r->permission))
			continue;
		if ((UW)addr < (UW)(r->start_addr))
			continue;
		if ((UW)addr < (UW)(r->start_addr) + r->max_size)
			return r;
	}

	return NULL;
}

PTE *copy_kernel_page_table(const PTE *src)
{
	char *directory = (char*)palloc();

	if (directory) {
		size_t n = (size_t)getDirectoryOffset((void*)MIN_KERNEL)
				* sizeof(PTE);

		memset(directory, 0, n);
		memcpy(directory + n, (char*)kern_p2v(src) + n, PAGE_SIZE - n);
	}

	return (PTE*)kern_v2p(directory);
}

void release_user_pages(PTE *directory)
{
	PTE *vdir = (PTE*)kern_p2v(directory);
	size_t n = (size_t)getDirectoryOffset((void*)MIN_KERNEL);
	size_t i;

	for (i = 0; i < n; i++)
		if (is_present(vdir[i])) {
			PTE *p = (PTE*)(vdir[i] & PAGE_ADDR_MASK);
			PTE *vp = (PTE*)(kern_p2v(p));
			size_t j;

			for (j = 0; j < PTE_PER_PAGE; j++)
				if (is_present(vp[j]))
					pfree((void*)(vp[j] & PAGE_ADDR_MASK));

			pfree(p);
		}

	pfree(directory);
}

ER copy_user_pages(PTE *dest, const PTE *src, size_t cnt)
{
	PTE *srcdir = (PTE*)kern_p2v(src);
	PTE *destdir = (PTE*)kern_p2v(dest);
	size_t n = (size_t)getDirectoryOffset((void*)MIN_KERNEL);
	size_t i;

	for (i = 0; i < n; i++) {
		PTE *srcp = (PTE*)(srcdir[i]);
		PTE *destp;
		size_t j;

		if (!is_present((PTE)srcp))
			continue;

		destp = (PTE*)(destdir[i]);
		if (is_present((PTE)destp))
			destp = (PTE*)((PTE)destp & PAGE_ADDR_MASK);

		else {
			destp = (PTE*)kern_v2p(palloc());
			if (!destp)
				return E_NOMEM;

			destdir[i] = calc_pte(destp, ATTR_USER);
		}

		srcp = (PTE*)(kern_p2v((PTE*)((PTE)srcp & PAGE_ADDR_MASK)));
		destp = (PTE*)(kern_p2v(destp));

		for (j = 0; j < PTE_PER_PAGE; j++) {
			char *p;

			if (!is_present(srcp[j]))
				continue;

			p = (char*)(destp[j]);
			if (is_present((PTE)p))
				p = (char*)((PTE)p & PAGE_ADDR_MASK);

			else {
				p = (char*)kern_v2p(palloc());
				if (!p)
					return E_NOMEM;

				destp[j] = calc_pte(p, ATTR_USER);
			}

			memcpy(kern_p2v(p),
					kern_p2v((char*)(srcp[j] & PAGE_ADDR_MASK)),
					PAGE_SIZE);

			if (!--cnt)
				return E_OK;
		}
	}

	return E_OK;
}
