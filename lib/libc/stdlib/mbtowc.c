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
#include <stdlib.h>


wchar_t append_body(wchar_t ch, const unsigned char *restrict p, const size_t n)
{
	size_t len;

	for (len = n; len > 0; len--) {
		if ((*++p >> 6) != 0x2)
			return 0;

		ch = (ch << 6) | (*p & 0x3f);
	}

	return ch;
}

int _mbtowc(wchar_t *restrict w, const unsigned char *restrict r, size_t n)
{
	wchar_t ch = 0;

	if (!(r[0] >> 7)) {
		ch = r[0];
		*w = ch;
		return (ch ? 1 : 0);
	}

	if ((r[0] >> 5) == 0x6) {
		if (n < 2)
			return (-1);

		ch = append_body(r[0] & 0x1f, r, 1);
		if (ch <= 0x7f)
			return (-1);

		*w = ch;
		return 2;
	}

	if ((r[0] >> 4) == 0xe) {
		if (n < 3)
			return (-1);

		ch = append_body(r[0] & 0xf, r, 2);
		if (ch <= 0x7ff)
			return (-1);

		*w = ch;
		return 3;
	}

	if ((r[0] >> 3) == 0x1e) {
		if (n < 4)
			return (-1);

		ch = append_body(r[0] & 0x7, r, 3);
		if ((ch <= 0xffff) || (ch > 0x10ffff))
			return (-1);

		*w = ch;
		return 4;
	}

	return (-1);
}

/**
 * convert UTF-8 character to UTF-32 character.
 */
int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n)
{
	if (!pwc)
		return mblen(s, n);
	
	if (!s)
		return 0;

	if (!n)
		return 0;

	return _mbtowc(pwc, (const unsigned char *restrict)s, n);
}
