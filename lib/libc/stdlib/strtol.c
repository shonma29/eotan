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
#include <ctype.h>
#include <errno.h>
#include <local.h>
#include <stdbool.h>
#include <stdlib.h>

#define BASE_MIN (2)
#define BASE_MAX (36)

#define BASE_OCT (8)
#define BASE_DEC (10)
#define BASE_HEX (16)

static inline bool is_hex(const unsigned char *r)
{
	return ((r[0] == '0')
			&& (toupper(r[1]) == 'X'));
}

static inline void set_endptr(char **restrict endptr, const unsigned char *str)
{
	if (endptr)
		*endptr = (char *) str;
}

static long get_num(const int, const int);


static long get_num(const int chr, const int base)
{
	long n;

	if ((chr >= '0')
			&& (chr <= '9'))
		n = chr - '0';
	else {
		int c = toupper(chr);
		if ((c >= 'A')
				&& (c <= 'Z'))
			n = c - 'A' + 10;
		else
			return (-1);
	}

	return ((base <= n) ? (-1) : n);
}

long strtol(const char *restrict str, char **restrict endptr, int base)
{
	bool minus = false;
	unsigned char *r = (unsigned char *) str;
	set_endptr(endptr, r);

	for (; isspace(*r); r++);

	switch (*r) {
	case '-':
		minus = true;
	case '+':
		r++;
	default:
		break;
	}

	if (base) {
		if ((base < BASE_MIN)
				|| (BASE_MAX < base)) {
			_set_local_errno(EINVAL);
			return 0;
		}

		if ((base == BASE_HEX)
				&& is_hex(r))
			r += 2;
	} else if (r[0] == '0') {
		if (toupper(r[1]) == 'X') {
			r += 2;
			base = BASE_HEX;
		} else {
			r++;
			base = BASE_OCT;
		}
	} else
		base = BASE_DEC;

	unsigned long v = 0;
	unsigned long max = minus ? LONG_MIN : LONG_MAX;
	for (long n; (n = get_num(*r, base)) >= 0; r++) {
		unsigned long y;
		if (__builtin_umull_overflow(v, base, &y)
				|| (y > max)
				|| ((unsigned long) n > max - y)) {
			_set_local_errno(ERANGE);
			v = max;
		} else
			v = y + (unsigned long) n;
	}

	set_endptr(endptr, r);
	return (minus ? (-v) : v);
}
