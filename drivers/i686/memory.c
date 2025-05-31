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
#include <sys/errno.h>
#include <sys/unistd.h>
#include <mpufunc.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <nerve/func.h>
#include "paging.h"

static inline uint32_t convert_attr(const int permission)
{
	return ((permission & W_OK) ? ATTR_USER : ATTR_USER_READ_ONLY);
}

static inline void copy_page(int32_t *dest, const int32_t *src)
{
	for (unsigned int i = 0; i < PAGE_SIZE / sizeof(int32_t); i++)
		dest[i] = src[i];
}

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

int copy_user_pages(void *dest, const void *src, const void *addr,
		const size_t n, const int permission)
{
	if (((uintptr_t) addr >= SUPERVISOR_START)
			|| (n > pages(SUPERVISOR_START - (uintptr_t) addr)))
		return EINVAL;

	uint32_t attr = convert_attr(permission);
	PDE *src_dir = kern_p2v(src);
	PDE *dest_dir = kern_p2v(dest);
	unsigned int offset = getPageOffset(addr);
	size_t rest = n;
	for (unsigned int i = getDirectoryOffset(addr); rest > 0; i++) {
		if (!is_present(src_dir[i]))
			return EFAULT;

		size_t max = PTE_PER_PAGE;
		if ((max - offset) > rest)
			max = offset + rest;

		PTE *dest_page = (PTE *) (dest_dir[i]);
		if (dest_page)
			dest_page = (PTE *) ((PDE) dest_page & PAGE_ADDR_MASK);
		else {
			dest_page = kern_v2p(kcall->palloc());
			if (!dest_page)
				return ENOMEM;

			dest_dir[i] = calc_pte(dest_page, ATTR_USER);
		}

		dest_page = kern_p2v(dest_page);
		PTE *src_page = kern_p2v((PTE *) (src_dir[i] & PAGE_ADDR_MASK));
		for (unsigned int j = offset; j < max; j++) {
			if (!is_present(src_page[j]))
				return EFAULT;

			if (dest_page[j])
				return EADDRINUSE;

			void *p = kern_v2p(kcall->palloc());
			if (!p)
				return ENOMEM;

			dest_page[j] = calc_pte(p, attr);
			copy_page(kern_p2v(p),
					kern_p2v((void *) (src_page[j]
							& PAGE_ADDR_MASK)));
		}

		rest -= max - offset;
		offset = 0;
	}

	return 0;
}

int attach_user_pages(void *dest, const void *src, const void *addr,
		const size_t n, const int permission)
{
	if (((uintptr_t) addr >= SUPERVISOR_START)
			|| (n > pages(SUPERVISOR_START - (uintptr_t) addr)))
		return EINVAL;

	uint32_t attr = convert_attr(permission);
	PDE *src_dir = kern_p2v(src);
	PDE *dest_dir = kern_p2v(dest);
	unsigned int offset = getPageOffset(addr);
	size_t rest = n;
	for (unsigned int i = getDirectoryOffset(addr); rest > 0; i++) {
		if (!is_present(src_dir[i]))
			return EFAULT;

		size_t max = PTE_PER_PAGE;
		if ((max - offset) > rest)
			max = offset + rest;

		PTE *dest_page = (PTE *) (dest_dir[i]);
		if (dest_page)
			dest_page = (PTE *) ((PDE) dest_page & PAGE_ADDR_MASK);
		else {
			dest_page = kern_v2p(kcall->palloc());
			if (!dest_page)
				return ENOMEM;

			dest_dir[i] = calc_pte(dest_page, ATTR_USER);
		}

		dest_page = kern_p2v(dest_page);
		PTE *src_page = kern_p2v((PTE *) (src_dir[i] & PAGE_ADDR_MASK));
		for (unsigned int j = offset; j < max; j++) {
			if (!is_present(src_page[j]))
				return EFAULT;

			if (dest_page[j])
				return EADDRINUSE;

			dest_page[j] = (src_page[j] & PAGE_ADDR_MASK) | attr;
		}

		rest -= max - offset;
		offset = 0;
	}

	return 0;
}

ER map_user_pages(PTE *dir, VP addr, size_t cnt, const int permission)
{
	uint32_t attr = convert_attr(permission);
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

				page[j] = calc_pte(p, attr);
//TODO reset page cache
			}
		}
	}
//TODO rollback when error
	return E_OK;
}

int unmap_user_pages(void *dir, const void *addr, const size_t n)
{
	if (((uintptr_t) addr >= SUPERVISOR_START)
			|| (n > pages(SUPERVISOR_START - (uintptr_t) addr)))
		return EINVAL;

	PDE *dest_dir = kern_p2v(dir);
	unsigned int offset = getPageOffset(addr);
	size_t rest = n;
	for (unsigned int i = getDirectoryOffset(addr); rest > 0; i++) {
		size_t max = PTE_PER_PAGE;
		if ((max - offset) > rest)
			max = offset + rest;

		if (!is_present(dest_dir[i])) {
			rest -= max - offset;
			offset = 0;
			continue;
		}

		PTE *dest_page = kern_p2v(
				(void *) (dest_dir[i] & PAGE_ADDR_MASK));
		bool exists_page = false;
		unsigned int j = 0;
		for (; j < offset; j++)
			if (is_present(dest_page[j])) {
				exists_page = true;
				break;
			}

		for (j = offset; j < max; j++) {
			PTE p = dest_page[j];
			if (is_present(p)) {
				dest_page[j] = 0;
				kcall->pfree((void *) (p & PAGE_ADDR_MASK));
			}
		}

		//TODO optimize
		if (!exists_page) {
			for (; j < PTE_PER_PAGE; j++)
				if (is_present(dest_page[j])) {
					exists_page = true;
					break;
				}

			if (!exists_page) {
				dest_dir[i] = 0;
				kcall->pfree(dest_page);
			}
		}

		rest -= max - offset;
		offset = 0;
	}

	return 0;
}

int detach_user_pages(void *dir, const void *addr, const size_t n)
{
	if (((uintptr_t) addr >= SUPERVISOR_START)
			|| (n > pages(SUPERVISOR_START - (uintptr_t) addr)))
		return EINVAL;

	PDE *dest_dir = kern_p2v(dir);
	unsigned int offset = getPageOffset(addr);
	size_t rest = n;
	for (unsigned int i = getDirectoryOffset(addr); rest > 0; i++) {
		size_t max = PTE_PER_PAGE;
		if ((max - offset) > rest)
			max = offset + rest;

		if (!is_present(dest_dir[i])) {
			rest -= max - offset;
			offset = 0;
			continue;
		}

		PTE *dest_page = kern_p2v(
				(void *) (dest_dir[i] & PAGE_ADDR_MASK));
		bool exists_page = false;
		unsigned int j = 0;
		for (; j < offset; j++)
			if (is_present(dest_page[j])) {
				exists_page = true;
				break;
			}

		for (j = offset; j < max; j++) {
			PTE p = dest_page[j];
			if (is_present(p))
				dest_page[j] = 0;
		}

		//TODO optimize
		if (!exists_page) {
			for (; j < PTE_PER_PAGE; j++)
				if (is_present(dest_page[j])) {
					exists_page = true;
					break;
				}

			if (!exists_page) {
				dest_dir[i] = 0;
				kcall->pfree(dest_page);
			}
		}

		rest -= max - offset;
		offset = 0;
	}

	return 0;
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
	if ((uintptr_t) from >= SUPERVISOR_START) {
		unsigned int offset = getOffset(to);
		if (bytes > PAGE_SIZE - offset)
			return E_PAR;

		void *p = getPageAddress(dir, to);
		if (!p)
			return E_PAR;

		memrcpy((void *) ((uintptr_t) p | offset), from, bytes);
		return 0;
	}

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
