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
#include <string.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include "paging.h"
#include "func.h"

static kcall_t *kcall = (kcall_t*)KCALL_ADDR;


PTE *copy_kernel_page_table(void)
{
	char *directory = (char*)(kcall->palloc());

	if (directory) {
		size_t n = (size_t)getDirectoryOffset((void*)SUPERVISOR_START)
				* sizeof(PTE);

		memset(directory, 0, n);
		memcpy(directory + n,
				(char*)kern_p2v((void*)KTHREAD_DIR_ADDR) + n,
				PAGE_SIZE - n);
		directory = (char*)kern_v2p(directory);
	}

	return (PTE*)directory;
}

ER copy_user_pages(PTE *dest, const PTE *src, size_t cnt)
{
	PTE *srcdir = (PTE*)kern_p2v(src);
	PTE *destdir = (PTE*)kern_p2v(dest);
	size_t n = (size_t)getDirectoryOffset((void*)SUPERVISOR_START);
	size_t i;

	for (i = 0; (i < n) && (cnt > 0); i++) {
		PTE *srcp = (PTE*)(srcdir[i]);
		PTE *destp;
		size_t j;

		if (!is_present((PTE)srcp)) {
			if (cnt <= PTE_PER_PAGE)
				break;

			cnt -= PTE_PER_PAGE;
			continue;
		}

		destp = (PTE*)(destdir[i]);
		if (is_present((PTE)destp))
			destp = (PTE*)((PTE)destp & PAGE_ADDR_MASK);

		else {
			destp = (PTE*)kern_v2p(kcall->palloc());
			if (!destp)
				return E_NOMEM;

			destdir[i] = calc_pte(destp, ATTR_USER);
		}

		srcp = (PTE*)(kern_p2v((PTE*)((PTE)srcp & PAGE_ADDR_MASK)));
		destp = (PTE*)(kern_p2v(destp));

		for (j = 0; (j < PTE_PER_PAGE) && (cnt > 0); cnt--, j++) {
			char *p;

			if (!is_present(srcp[j]))
				continue;

			p = (char*)(destp[j]);
			if (is_present((PTE)p))
				p = (char*)((PTE)p & PAGE_ADDR_MASK);

			else {
				p = (char*)kern_v2p(kcall->palloc());
				if (!p)
					return E_NOMEM;

				destp[j] = calc_pte(p, ATTR_USER);
			}

			memcpy(kern_p2v(p),
					kern_p2v((char*)(srcp[j] & PAGE_ADDR_MASK)),
					PAGE_SIZE);
		}
	}

	return E_OK;
}
