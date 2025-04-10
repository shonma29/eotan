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
#include <mpufunc.h>
#include <mpu/memory.h>
#include <nerve/func.h>
#include <nerve/thread.h>


ER memcpy_k2u(thread_t *th, void *to, const void *from, const size_t bytes)
{
	if (is_kthread(th)) {
		memcpy(to, from, bytes);
		return E_OK;
	}

	PTE *dir = (PTE *) kern_p2v(th->mpu.cr3);
	void *p = getPageAddress(dir, to);
	if (p) {
		char *w = (char *) to;
		char *r = (char *) from;
		size_t rest = bytes;
		size_t offset = getOffset(w);
		size_t len = PAGE_SIZE - offset;

		p += offset;

		do {
			len = (rest < len) ? rest : len;
			memcpy(p, r, len);

			if (!(rest -= len))
				return E_OK;

			w += len;
			r += len;
			len = PAGE_SIZE;
		} while((p = getPageAddress(dir, w)));
	}

	return E_PAR;
}

ER memcpy_u2k(thread_t *th, void *to, const void *from, const size_t bytes)
{
	if (is_kthread(th)) {
		memcpy(to, from, bytes);
		return E_OK;
	}

	PTE *dir = (PTE *) kern_p2v(th->mpu.cr3);
	void *p = getPageAddress(dir, from);
	if (p) {
		char *w = (char *) to;
		char *r = (char *) from;
		size_t rest = bytes;
		size_t offset = getOffset(r);
		size_t len = PAGE_SIZE - offset;

		p += offset;

		do {
			len = (rest < len) ? rest : len;
			memcpy(w, p, len);

			if (!(rest -= len))
				return E_OK;

			w += len;
			r += len;
			len = PAGE_SIZE;
		} while ((p = getPageAddress(dir, r)));
	}

	return E_PAR;
}
