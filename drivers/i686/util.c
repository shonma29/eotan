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
