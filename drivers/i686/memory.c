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
#include <stdint.h>
#include <string.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include "func.h"
#include "mpufunc.h"
#include "paging.h"

static void memrcpy(char *, const char *, const size_t);


PTE *copy_kernel_page_table(void)
{
	char *directory = (char *) (kcall->palloc());

	if (directory) {
		size_t n = (size_t) getDirectoryOffset(
				(void *) SUPERVISOR_START) * sizeof(PTE);

		memset(directory, 0, n);
		memcpy(directory + n,
				(char *) kern_p2v((void *) KTHREAD_DIR_ADDR)
						+ n,
				PAGE_SIZE - n);
		directory = (char *) kern_v2p(directory);
	}

	return (PTE *) directory;
}

ER copy_user_pages(PTE *dest, const PTE *src, size_t cnt)
{
	PTE *srcdir = (PTE *) kern_p2v(src);
	PTE *destdir = (PTE *) kern_p2v(dest);
	size_t n = (size_t) getDirectoryOffset((void *) SUPERVISOR_START)
			* PTE_PER_PAGE;
	size_t i;

	if (n > cnt)
		n = cnt;

	for (i = 0; n > 0; i++) {
		PTE *srcp = (PTE *) (srcdir[i]);
		PTE *destp;
		size_t j;

		if (!is_present((PTE) srcp)) {
			if (n <= PTE_PER_PAGE)
				break;

			n -= PTE_PER_PAGE;
			continue;
		}

		destp = (PTE *) (destdir[i]);
		if (is_present((PTE) destp))
			destp = (PTE *) ((PTE) destp & PAGE_ADDR_MASK);

		else {
			destp = (PTE *) kern_v2p(kcall->palloc());
			if (!destp)
				return E_NOMEM;

			destdir[i] = calc_pte(destp, ATTR_USER);
		}

		srcp = (PTE *) (kern_p2v((PTE *) ((PTE) srcp
				& PAGE_ADDR_MASK)));
		destp = (PTE *) (kern_p2v(destp));

		for (j = 0; (j < PTE_PER_PAGE) && (n > 0); n--, j++) {
			char *p;

			if (!is_present(srcp[j]))
				continue;

			p = (char *) (destp[j]);
			if (is_present((PTE) p))
				p = (char *) ((PTE) p & PAGE_ADDR_MASK);

			else {
				p = (char *) kern_v2p(kcall->palloc());
				if (!p)
					return E_NOMEM;

				destp[j] = calc_pte(p, ATTR_USER);
			}

			memcpy(kern_p2v(p),
					kern_p2v((char *) (srcp[j]
							& PAGE_ADDR_MASK)),
					PAGE_SIZE);
		}
	}

	return E_OK;
}

ER map_user_pages(PTE *dir, VP addr, size_t cnt)
{
	size_t n = (size_t) getDirectoryOffset((void *) SUPERVISOR_START)
			* PTE_PER_PAGE;
	size_t i = (((unsigned int) addr) >> (BITS_PAGE + BITS_OFFSET))
			& MASK_PAGE;
	size_t j = (((unsigned int) addr) >> BITS_OFFSET) & MASK_PAGE;

//TODO check range
	dir = (PTE *) kern_p2v(dir);

	if (n > cnt)
		n = cnt;

	for (; n > 0; j = 0, i++) {
		PTE *page = (PTE *) (dir[i]);

		if (is_present((PTE) page))
//TODO check permission
			page = (PTE *) ((PTE) page & PAGE_ADDR_MASK);

		else {
			page = (PTE *) kern_v2p(kcall->palloc());
			if (!page)
				return E_NOMEM;

			dir[i] = calc_pte(page, ATTR_USER);
		}

		page = (PTE *) kern_p2v(page);

		for (; (j < PTE_PER_PAGE) && (n > 0); n--, j++) {
			char *p = (char *) (page[j]);
			if (is_present((PTE) p)) {
//TODO check permission
				p = (char *) ((PTE) p & PAGE_ADDR_MASK);
//				memset(kern_p2v(p), 0, PAGE_SIZE);

			} else {
				p = (char *) kern_v2p(kcall->palloc());
				if (!p)
					return E_NOMEM;

				page[j] = calc_pte(p, ATTR_USER);
//TODO reset page cache
			}
		}
	}
//TODO rollback when error
	return E_OK;
}

ER unmap_user_pages(PTE *dir, VP addr, size_t cnt)
{
	size_t n = (size_t) getDirectoryOffset((void *) SUPERVISOR_START)
			* PTE_PER_PAGE;
	size_t i = (((unsigned int) addr) >> (BITS_PAGE + BITS_OFFSET))
			& MASK_PAGE;
	size_t j = (((unsigned int) addr) >> BITS_OFFSET) & MASK_PAGE;

//TODO check range
	dir = (PTE *) kern_p2v(dir);

	if (n > cnt)
		n = cnt;

	for (; n > 0; j = 0, i++) {
		PTE *page = (PTE *) (dir[i]);
		if (!is_present((PTE) page)) {
			//TODO really?
			if (n <= PTE_PER_PAGE)
				break;

			//TODO really?
			n -= PTE_PER_PAGE;
			continue;
		}

		page = kern_p2v((void *) (((PTE) page) & PAGE_ADDR_MASK));

		size_t zero_count = 0;
		unsigned int k = 0;
		for (; k < j; k++)
			if (!(page[k]))
				zero_count++;

		for (; (j < PTE_PER_PAGE) && (n > 0); n--, j++) {
			PTE p = page[j];
			if (is_present(p)) {
				page[j] = 0;
				kcall->pfree((void *) (p & PAGE_ADDR_MASK));
//TODO reset page cache
			}
		}

		zero_count += j - k;
		for (k = j; k < PTE_PER_PAGE; k++)
			if (!(page[k]))
				zero_count++;

		if (zero_count == PTE_PER_PAGE) {
			dir[i] = 0;
			kcall->pfree((void *) (((PTE) page) & PAGE_ADDR_MASK));
		}
	}

	return E_OK;
}

static void memrcpy(char *to, const char *from, const size_t bytes)
{
	char *w = to;
	char *r = (char *) from;
	size_t len;

	for (len = bytes; len; len--)
		*--w = *--r;
}

ER move_stack(const PTE *page_table, void *to, const void *from,
		const size_t bytes)
{
	PTE *dir = (PTE *) kern_p2v(page_table);
	size_t left = bytes;
	size_t roffset = getOffset((void *) ((uintptr_t) from + left));
	size_t woffset = PAGE_SIZE - roffset;
	void *q = getPageAddress(dir, (void *) ((uintptr_t) from + left - 1));
	if (!q)
		return E_PAR;

	while (left) {
		void *p = getPageAddress(dir,
				(void *) ((uintptr_t) to + left - 1));
		if (!p)
			return E_PAR;

		if (roffset) {
			size_t len = (roffset > left) ? left : roffset;

			memrcpy(p + PAGE_SIZE, q + roffset, len);
			left -= len;
			if (!left)
				break;
		}

		q = getPageAddress(dir, (void *) ((uintptr_t) from + left - 1));
		if (q) {
			size_t len = (woffset > left) ? left : woffset;

			memrcpy(p + woffset, q + PAGE_SIZE, len);
			left -= len;
		}
		else
			return E_PAR;
	}

	return E_OK;
}
