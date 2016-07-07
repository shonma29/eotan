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

static void _inc(Frequency *freq, const unsigned int ch, const int n);


static void _inc(Frequency *freq, const unsigned int ch, const int n)
{
	if (ch > 0) {
		int c = ch;

		for (; c < CHR_RANGE; c += (c & (- c)))	freq->count[c] += n;
	} else
		freq->count[0] += n;

	freq->sum += n;
}

void frequency_initialize(Frequency *freq)
{
	int i;

	freq->size = CHR_RANGE;
	freq->sum = 0;

	for (freq->mid = 1; freq->mid < CHR_RANGE / 2;)	freq->mid <<= 1;
	for (i = 0; i < CHR_RANGE; i++)	freq->count[i] = 0;
	for (i = 0; i < CHR_RANGE; i++)	_inc(freq, i, 1);
}

unsigned int frequency_count(Frequency *freq, const unsigned int ch)
{
	unsigned int n = freq->count[ch];

	if ((ch > 0)
			 && !(ch & 1)) {
		unsigned int i = ch - 1;
		unsigned int j = ch & i;

		for (; i != j; i &= (i - 1))
			n -= freq->count[i];
	}

	return n;
}

void frequency_update(Frequency *freq, const unsigned int ch)
{
	_inc(freq, ch, FREQ_INC);

	if (freq->sum >= FREQ_LIMIT) {
		unsigned int i;

		for (i = 0; i < CHR_RANGE; i++) {
			unsigned int n = frequency_count(freq, i) >> 1;

			if (n > 0)
				_inc(freq, i, - n);
		}
	}
}
