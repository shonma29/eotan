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

static unsigned int _calc(Frequency *freq, const unsigned int ch);


static unsigned int _calc(Frequency *freq, const unsigned int ch) {
	unsigned int n = 0;

	if (ch > 0) {
		unsigned int i;

		for (n = freq->count[0], i = ch - 1; i > 0; i &= (i - 1))
			n += freq->count[i];
	}

	return n;
}

int frequency_encode(Frequency *freq, RangeCoder *rc, const unsigned int ch)
{
	int result;
	unsigned int tmp = rc->range / freq->sum;
	unsigned int prev_low = rc->low;

	rc->low += _calc(freq, ch) * tmp;
	rc->carry = (rc->low < prev_low);
	rc->range = frequency_count(freq, ch) * tmp;

	if ((result = rc_encode_normalize(rc)))
		return result;

	frequency_update(freq, ch);

	return ERR_OK;
}
