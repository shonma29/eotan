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


int rc_decode_normalize(RangeCoder *rc)
{
	while (rc->range < MIN_RANGE) {
		int c = rc->get(rc);

		if (c == EOF)
			return ERR_FILE;

		rc->low = (rc->low << 8) + c;
		rc->range <<= 8;
	}

	return ERR_OK;
}

int rc_decode(Frequency *freq, RangeCoder *rc)
{
	int c;

	if ((c = rc->get(rc)) == EOF)
		return ERR_FILE;
	else
		rc->low = c << 24;

	if ((c = rc->get(rc)) == EOF)
		return ERR_FILE;
	else
		rc->low |= c << 16;

	if ((c = rc->get(rc)) == EOF)
		return ERR_FILE;
	else
		rc->low |= c << 8;

	if ((c = rc->get(rc)) == EOF)
		return ERR_FILE;
	else
		rc->low |= c;

	for (;;) {
		c = frequency_decode(freq, rc);

		if (c == EOF)
			return ERR_FILE;

		if (c == CHR_EOS)
			break;

		if (rc->put(c, rc) == EOF)
			return ERR_FILE;
	}

	return ERR_OK;
}
