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
#include <stdbool.h>
#include <sys/types.h>
#include <mpu/bits.h>

#define MAX_INT_COLUMN (10)
#define INT_BIT ((CHAR_BIT) * sizeof(int))
#ifdef _LP64
#define MAX_LONG_COLUMN (20)
#define LONG_BIT ((CHAR_BIT) * sizeof(long))
#endif

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
#ifdef _LP64
static void _putld(State *, const long);
#endif
static void _putx(State *, const unsigned int);
#ifdef _LP64
static void _putlx(State *, const unsigned long);
#endif
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
	for (char c; (c = *str); str++)
		_putchar(s, c);
}

static void _putd(State *s, const int v)
{
	unsigned int u;
	if (v < 0) {
		_putchar(s, '-');
		u = -v;
	} else
		u = v;

	char buf[MAX_INT_COLUMN + 1];
	char *p = &buf[sizeof(buf) - 1];
	*p = '\0';

	do {
		*--p = (u % 10) + '0';
	} while (u /= 10);

	_puts(s, p);
}
#ifdef _LP64
static void _putld(State *s, const long v)
{
	unsigned long u;
	if (v < 0) {
		_putchar(s, '-');
		u = -v;
	} else
		u = v;

	char buf[MAX_LONG_COLUMN + 1];
	char *p = &buf[sizeof(buf) - 1];
	*p = '\0';

	do {
		*--p = (u % 10) + '0';
	} while (u /= 10);

	_puts(s, p);
}
#endif
static void _putx(State *s, const unsigned int u)
{
	int shift = count_nlz(u);
	if (shift < INT_BIT) {
		shift &= 0x1c;
		do {
			int c = (u >> shift) & 0xf;
			_putchar(s, c + ((c >= 10) ? ('a' - 10) : '0'));
		} while ((shift -= 4) >= 0);
	} else
		_putchar(s, '0');
}
#ifdef _LP64
static void _putlx(State *s, const unsigned long u)
{
	long shift = count_nlz(u);
	if (shift < LONG_BIT) {
		shift &= 0x1c;
		do {
			int c = (u >> shift) & 0xf;
			_putchar(s, c + ((c >= 10) ? ('a' - 10) : '0'));
		} while ((shift -= 4) >= 0);
	} else
		_putchar(s, '0');
}
#endif
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
	case 'l': {
			char c2 = *(s->format)++;
			switch (c2) {
			case 'd':
#ifdef _LP64
				_putld(s, va_arg(*(s->ap), long));
#else
				_putd(s, va_arg(*(s->ap), long));
#endif
				break;
			case 'x':
#ifdef _LP64
				_putlx(s, va_arg(*(s->ap), unsigned long));
#else
				_putx(s, va_arg(*(s->ap), unsigned long));
#endif
				break;
			default:
				_putchar(s, '%');
				_putchar(s, c);
				_putchar(s, c2);
				break;
			}
		}
		break;
	case 'p':
		_putchar(s, '0');
		_putchar(s, 'x');
		_putx(s, va_arg(*(s->ap), uintptr_t));
		break;
	case 'x':
		_putx(s, va_arg(*(s->ap), unsigned int));
		break;
	case 's':
		_puts(s, va_arg(*(s->ap), char *));
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
