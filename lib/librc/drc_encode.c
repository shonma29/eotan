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

static int _put_buf(RangeCoder *rc, const unsigned char ch, const size_t n);
static int _finish(RangeCoder *);


static int _put_buf(RangeCoder *rc, const unsigned char ch, const size_t n)
{
	unsigned int i;

	if (rc->put(rc->buf, rc) == EOF)
		return ERR_FILE;

	for (i = 0; i < n; i++)
		if (rc->put(ch, rc) == EOF)
			return ERR_FILE;

	return ERR_OK;
}

int rc_encode_normalize(RangeCoder *rc)
{
	if (rc->carry) {
		rc->carry = false;
		rc->buf++;

		if (rc->count > 0) {
			int result = _put_buf(rc, 0, rc->count - 1);

			if (result)
				return result;

			rc->buf = 0;
			rc->count = 0;
		}
	}

	while (rc->range < MIN_RANGE) {
		if (rc->low < (unsigned int)(0xff << SHIFT)) {
			int result = _put_buf(rc, 0xff, rc->count);

			if (result)
				return result;

			rc->buf = rc->low >> SHIFT;
			rc->count = 0;
		} else
			rc->count++;

		rc->low <<= 8;
		rc->range <<= 8;
	}

	return ERR_OK;
}

static int _finish(RangeCoder *rc)
{
	int result;
	unsigned int ch = 0xff;

	if (rc->carry) {
		rc->carry = false;
		rc->buf++;
		ch = 0;
	}

	if ((result = _put_buf(rc, ch, rc->count)))
		return result;

	if (rc->put(rc->low >> 24, rc) == EOF)
		return ERR_FILE;

	if (rc->put((rc->low >> 16) & 0xff, rc) == EOF)
		return ERR_FILE;

	if (rc->put((rc->low >> 8) & 0xff, rc) == EOF)
		return ERR_FILE;

	if (rc->put(rc->low & 0xff, rc) == EOF)
		return ERR_FILE;

	return ERR_OK;
}

int rc_encode(Frequency *freq, RangeCoder *rc)
{
	int result;
	unsigned int ch;

	while ((ch = rc->get(rc)) != EOF)
		if ((result = frequency_encode(freq, rc, ch)))
			return result;

	if ((result = frequency_encode(freq, rc, CHR_EOS)))
		return result;

	return _finish(rc);
}
