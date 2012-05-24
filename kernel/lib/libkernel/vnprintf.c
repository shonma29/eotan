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
#include "../../../include/stdarg.h"
#include "../../../include/itron/types.h"

#define MAX_INT_COLUMN (10)
#define MAX_INT_BITS (32)

typedef struct _State {
	W (*handler)(struct _State*);
	B *format;
	va_list ap;
	void (*out)(const B);
	W len;
} State;

static void _putchar(State *s, const B c);
static void _puts(State *s, const B *str);
static void _putd(State *s, const W v);
static void _puth(State *s, const W v);
static W _immediate(State *s);
static W _format(State *s);
static W _escape(State *s);


static void _putchar(State *s, const B c) {
	s->len++;
	s->out(c);
}

static void _puts(State *s, const B *str) {
	B c;

	for (; c = *str; str++)	_putchar(s, c);
}

static void _putd(State *s, const W v) {
	B buf[MAX_INT_COLUMN];
	B *p = &buf[sizeof(buf) - 1];
	W x = v;

	if (x < 0) {
		_putchar(s, '-');
		x = -x;
	}

	for (*p-- = '\0';; p--) {
		*p = (x % 10) + '0';

		if (!(x /= 10)) {
			_puts(s, p);
			break;
		}
	}
}

static void _puth(State *s, const W x) {
	W shift;

	for (shift = MAX_INT_BITS - 4; shift >= 0; shift -= 4) {
		W c = (x >> shift) & 0xf;

		_putchar(s, c + ((c >= 10)? ('a' - 10):'0'));
	}
}

static W _immediate(State *s) {
	B c = *(s->format)++;

	switch (c) {
	case '\0':
		return FALSE;
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

	return TRUE;
}

static W _format(State *s) {
	B c = *(s->format)++;

	switch (c) {
	case '\0':
		_putchar(s, '%');
		return FALSE;
	case 'c':
		_putchar(s, va_arg(s->ap, char));
		break;
	case 'd':
		_putd(s, va_arg(s->ap, int));
		break;
	case 'p':
		_putchar(s, '0');
		_putchar(s, 'x');
	case 'x':
		_puth(s, va_arg(s->ap, int));
		break;
	case 's':
		_puts(s, va_arg(s->ap, char*));
		break;
	default:
		_putchar(s, '%');
		_putchar(s, c);
		break;
	}

	s->handler = _immediate;

	return TRUE;
}

static W _escape(State *s) {
	B c = *(s->format)++;

	switch (c) {
	case '\0':
		_putchar(s, '\\');
		return FALSE;
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
	return TRUE;
}

W vnprintf(void (*out)(B), B *format, va_list ap) {
	State s = { _immediate, format, ap, out, 0 };

	while (s.handler(&s));

	return s.len;
}
