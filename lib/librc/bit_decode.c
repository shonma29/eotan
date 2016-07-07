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
#include "rangecoder.h"
#include "bit.h"


int frequency_decode(Frequency *freq, RangeCoder *rc)
{
	int result;
	unsigned int tmp = rc->range / freq->sum;
	unsigned int value = rc->low / tmp;
	unsigned int n = 0;
	unsigned int i = 0;

	if (freq->count[0] <= value) {
		unsigned int h = freq->mid;

		for (n = freq->count[0]; h > 0; h >>= 1) {
			unsigned int q = i + h;

			if ((q < CHR_RANGE)
					&& (n + freq->count[q] <= value)) {
				n += freq->count[q];
				i = q;
			}
		}

		i++;
	}

	rc->low -= n * tmp;
	rc->range = frequency_count(freq, i) * tmp;

	if ((result = rc_decode_normalize(rc)))
		return EOF;

	frequency_update(freq, i);

	return i;
}
