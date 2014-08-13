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
#include <mpu/memory.h>
#include <thread.h>
#include "mpufunc.h"

static bool copy(UB *, UB *, size_t *);


ER_UINT ncpy_from(thread_t *th, void *to, const void *from, const size_t bytes)
{
	PTE *dir = (PTE*)kern_p2v(th->mpu.cr3);
	UB *w = (UB*)to;
	UB *r = (UB*)from;
	size_t left = bytes;
	size_t offset = getOffset(r);

	while (left) {
		void *p = getPageAddress(dir, r);
		size_t len;
		bool done;
	
		if (!p)
			return E_PAR;

		len = PAGE_SIZE - offset;
		if (len > left)
			len = left;

		done = copy(w, p + offset, &len);
		left -= len;

		if (done)
			break;

		offset = 0;
		w += len;
		r += len;
	}

	return bytes - left;
}

static bool copy(UB *dest, UB *src, size_t *len)
{
	bool result = false;
	size_t left;

	for (left = *len; left > 0; left--) {
		UB c = *src++;

		*dest++ = c;
		if (!c) {
			result = true;
			break;
		}
	}

	*len -= left;
	return result;
}
