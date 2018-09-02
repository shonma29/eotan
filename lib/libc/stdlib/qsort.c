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
#include <stdlib.h>
#include <string.h>

#define THRESHOLD (50)

static inline int min(const int a, const int b)
{
	return (a < b)? a:b;
}

static inline void *getAddr(void *base, size_t size, int n)
{
	return (void*)((uintptr_t)base + size * n);
}

static inline void *addAddr(void *base, int offset)
{
	return (void*)((uintptr_t)base + offset);
}

static void merge(void *base, int low, int high, int range, void *work,
		size_t size, int (*compar)(const void *, const void *));
static void insert(void *base, int low, int high, void *work,
		size_t size, int (*compar)(const void *, const void *));


void qsort(void *base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *))
{
	void *work = (void *)malloc(nmemb * size);

	for (int range = 1; range < nmemb; range <<= 1) {
		int pair = range << 1;
		for (int i = 0; i < nmemb; i += pair) {
			int high = min(i + pair - 1, nmemb - 1);
			if (high - i < THRESHOLD)
				insert(base, i, high, work, size, compar);
			else
				merge(base, i, high, range - 1, work,
						size, compar);
		}
	}

	free(work);
}

static void merge(void *base, int low, int high, int range, void *work,
		size_t size, int (*compar)(const void *, const void *))
{
	if (low < high) {
		int i = range + 1;
		memcpy(work, getAddr(base, size, low), size * i);

		void *to = getAddr(base, size, low);
		void *w = getAddr(work, size, 0);
		void *b = getAddr(base, size, i);
		int j;
		for (j = 0; j <= range;) {
			if (compar(w, b) <= 0) {
				memcpy(to, w, size);
				to = addAddr(to, size);
				w = addAddr(w, size);
				j++;
			} else {
				memcpy(to, b, size);
				to = addAddr(to, size);
				b = addAddr(b, size);
				i++;

				if (i > high)
					break;
			}
		}

		memcpy(to, getAddr(work, size, j), size * (range - j + 1));
	}
}

static void insert(void *base, int low, int high, void *work,
		size_t size, int (*compar)(const void *, const void *))
{
	for (int i = low + 1; i <= high; i++) {
		memcpy(work, getAddr(base, size, i), size);

		int j;
		for (j = i - 1; j >= low; j--) {
			void *p = getAddr(base, size, j);
			if (compar(p, work) <= 0)
				break;

			memcpy(addAddr(p, size), p, size);
		}

		memcpy(getAddr(base, size, j + 1), work, size);
	}
}
