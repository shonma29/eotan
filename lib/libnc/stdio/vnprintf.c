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
#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>

#define MAX_INT_COLUMN (10)
#define INT_BIT ((CHAR_BIT) * sizeof(int))

typedef struct _State {
	bool (*handler)(struct _State*);
	char *format;
	va_list *ap;
	void (*out)(const char);
	int len;
} State;

static void _putchar(State *, const char);
static void _puts(State *, const char *);
static void _putd(State *, const int);
static void _puth(State *, const int);
static bool _immediate(State *);
static bool _format(State *);
static bool _escape(State *);


static void _putchar(State *s, const char c)
{
	s->len++;
	s->out(c);
}

static void _puts(State *s, const char *str)
{
	char c;

	for (; (c = *str); str++)	_putchar(s, c);
}

static void _putd(State *s, const int v)
{
	char buf[MAX_INT_COLUMN];
	char *p = &buf[sizeof(buf) - 1];
	int x = v;
	int carry = 0;

	if (x < 0) {
		unsigned int y = (unsigned int)x;

		if ((unsigned int)y == INT_MIN) {
			carry = 1;
			x = (int)(y + 1);
		}

		_putchar(s, '-');
		x = -x;
	}

	for (*p-- = '\0';; p--) {
		*p = (x % 10) + carry + '0';
		carry = 0;

		if (!(x /= 10)) {
			_puts(s, p);
			break;
		}
	}
}

static void _puth(State *s, const int x)
{
	bool show = false;

	for (int shift = INT_BIT - 4; shift >= 0; shift -= 4) {
		int c = (x >> shift) & 0xf;
		if (show || c) {
			_putchar(s, c + ((c >= 10) ? ('a' - 10) : '0'));
			show = true;
		}
	}

	if (!show)
		_putchar(s, '0');
}

static bool _immediate(State *s)
{
	char c = *(s->format)++;

	switch (c) {
	case '\0':
		return false;
	case '%':
		s->handler = _format;
		break;
	case '\\':
		s->handler = _escape;
		break;
	default:
		_putchar(s, c);
		break;
	}

	return true;
}

static bool _format(State *s)
{
	char c = *(s->format)++;

	switch (c) {
	case '\0':
		_putchar(s, '%');
		return false;
	case 'c':
		_putchar(s, va_arg(*(s->ap), int) & 0xff);
		break;
	case 'd':
		_putd(s, va_arg(*(s->ap), int));
		break;
	case 'p':
		_putchar(s, '0');
		_putchar(s, 'x');
	case 'x':
		_puth(s, va_arg(*(s->ap), int));
		break;
	case 's':
		_puts(s, va_arg(*(s->ap), char*));
		break;
	default:
		_putchar(s, '%');
		_putchar(s, c);
		break;
	}

	s->handler = _immediate;

	return true;
}

static bool _escape(State *s)
{
	char c = *(s->format)++;

	switch (c) {
	case '\0':
		_putchar(s, '\\');
		return false;
	case 'n':
		c = '\n';
		break;
	case 'r':
		c = '\r';
		break;
	case 't':
		c = '\t';
		break;
	default:
		break;
	}

	_putchar(s, c);
	s->handler = _immediate;
	return true;
}

int vnprintf(void (*out)(char), char *format, va_list *ap)
{
	State s = { _immediate, format, ap, out, 0 };

	while (s.handler(&s));

	return s.len;
}
