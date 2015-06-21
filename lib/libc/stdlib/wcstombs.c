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
#include <string.h>

extern int _wctomb(unsigned char *, const wchar_t);
static int count(wchar_t);


static int count(const wchar_t ch)
{
	if (ch <= 0x7f)
		return 1;
		
	else if (ch <= 0x7ff)
		return 2;

	else if (ch <= 0xffff)
		return 3;

	else if (ch <= 0x10ffff)
		return 4;

	else
		return -1;
}

/**
 * convert UTF-32 string to UTF-8 string.
 */
size_t wcstombs(char *restrict s, const wchar_t *restrict pwcs, size_t n)
{
	if (s) {
		size_t left = n;

		for (; *pwcs; pwcs++) {
			int m;

			if (left >= MB_CUR_MAX) {
				m = _wctomb((unsigned char*)s, *pwcs);
				if (m < 0)
					return (size_t)m;

			} else {
				unsigned char buf[MB_CUR_MAX];

				m = _wctomb(buf, *pwcs);
				if (m < 0)
					return (size_t)m;

				if (left < m) {
					memcpy(s, buf, left);
					return n;
				}

				memcpy(s, buf, m);
			}

			left -= m;
			s += m;
		}

		*s = '\0';
		return (n - left);

	} else {
		size_t len = 0;

		for (; *pwcs; pwcs++) {
			int m = count(*pwcs);

			if (m < 0)
				return (size_t)m;

			len += m;
		}

		return len;
	}
}
