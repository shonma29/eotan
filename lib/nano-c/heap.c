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
#include "set/heap.h"

void heap_initialize(heap_t *h, const int max, int *buf) {
	h->cnt = 0;
	h->max = max;
	h->buf = buf;
}

int heap_enqueue(heap_t *h, const int val) {
	int c = h->cnt;

	if (c >= h->max)	return HEAP_FULL;

	for (h->buf[c] = val; c > 0;) {
		int p = (c - 1) >> 1;
		int v1 = h->buf[c];
		int v2 = h->buf[p];

		if (v1 < v2) {
			h->buf[p] = v1;
			h->buf[c] = v2;
			c = p;
		}
		else	break;
	}

	h->cnt++;
	return HEAP_OK;
}

int heap_dequeue(heap_t *h) {
	int val;

	if (h->cnt) {
		int p;
		int c;

		val = h->buf[0];
		h->buf[0] = h->buf[--h->cnt];

		for (p = 0; (c = (p << 1) + 1) < h->cnt;) {
			int v1 = h->buf[c];
			int v2;
			
			if (c + 1 < h->cnt) {
				v2 = h->buf[c + 1];
				if (v1 > v2) {
					c++;
					v1 = v2;
				}
			}

			v2 = h->buf[p];
			if (v1 < v2) {
				h->buf[p] = v1;
				h->buf[c] = v2;
				p = c;
			}
			else	break;
		}
	}
	else	val = HEAP_EMPTY;

	return val;
}
