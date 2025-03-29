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
#include <mpufunc.h>


ER scattered_copy_u2k(const thread_t *th, const size_t chunk_size,
		copy_range_t *ranges, const size_t n)
{
	PTE *dir = (PTE *) kern_p2v(th->mpu.cr3);
	for (int i = 0; i < n; i++) {
		copy_range_t *range = &(ranges[i]);
		if (!(range->fragment_size))
			continue;

		for (int j = 0; j < range->num_of_fragments; j++) {
			const char *r = range->from;
			void *p = getPageAddress(dir, r);
			if (!p)
				return E_PAR;

			char *w = range->to;
			size_t rest = range->fragment_size;
			size_t offset = getOffset(r);
			size_t len = PAGE_SIZE - offset;
			p = &(p[offset]);

			for (;;) {
				len = (rest < len) ? rest : len;
				memcpy(w, p, len);

				if (!(rest -= len))
					break;

				r += len;
				w += len;
				len = PAGE_SIZE;
				p = getPageAddress(dir, r);
				if (!p)
					return E_PAR;
			}

			range->from += range->chunk_size;
			range->to += chunk_size;
		}
	}

	return E_OK;
}
