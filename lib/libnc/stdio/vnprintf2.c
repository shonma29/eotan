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

#ifdef USE_FLOAT
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mpu/ieee754.h>
#endif

#define MAX_INT_COLUMN (10)
#define INT_BIT ((CHAR_BIT) * sizeof(int))

#ifdef USE_FLOAT
#define NUMBER_MIN_UNITS (4)
#define NUMBER_CARRY_BIT (INT_BIT - 1)
#define NUMBER_CARRY_MASK ((uint32_t) (1 << NUMBER_CARRY_BIT))
#define NUMBER_VALUE_MASK ((uint32_t) (NUMBER_CARRY_MASK - 1))

#define B64_SIGNIFICANT_MASK (((uint64_t) 1 << B64_SIGNIFICANT_BITS) - 1)

typedef struct {
	bool minus;
	size_t len;
	size_t max;
	uint32_t *buf;
} number_t;
#endif

typedef struct _State {
	bool (*handler)(struct _State *);
	const char *format;
	va_list ap;
	int (*out)(const char, void *);
	void *env;
	int len;
} State;

static void _putchar(State *, const char);
static void _puts(State *, const char *);
static void _putd(State *, const int, const int);
static void _puth(State *, const int);

#ifdef USE_FLOAT
static uint32_t power5[] = {
	5, 25, 125, 625, 3125, 15625, 78125, 390625,
	1953125, 9765625, 48828125, 244140625, 1220703125
};
#define NUM_OF_POWER5 (sizeof(power5) / sizeof(power5[0]))

static number_t *number_create(const uint32_t *, const size_t,
		const bool);
static void number_destroy(number_t *);
static int number_multiply(number_t *, const uint32_t);
static int number_shift(number_t *, const int);
static int number_divide(number_t *, const uint32_t);
static void putdouble(State *, const bool, uint64_t, const int);
static void _putf(State *, const double);
#endif

static bool _immediate(State *);
static bool _format(State *);
static bool _escape(State *);


static void _putchar(State *s, const char c)
{
	if (s->out(c, s->env) >= 0)
		s->len++;
}

static void _puts(State *s, const char *str)
{
	char c;
	for (; (c = *str); str++)
		_putchar(s, c);
}

static void _putd(State *s, const int v, const int radix)
{
	int carry = 0;
	int x = v;
	if (x < 0) {
		unsigned int y = (unsigned int) x;
		if (y == (unsigned int) INT_MIN) {
			carry = 1;
			x = (int) (y + 1);
		}

		_putchar(s, '-');
		x = -x;
	}

	char buf[MAX_INT_COLUMN];
	char *p = &buf[sizeof(buf) - 1];
	for (*p-- = '\0';; p--) {
		*p = (x % radix) + carry + '0';
		carry = 0;

		if (!(x /= radix)) {
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

#ifdef USE_FLOAT
static number_t *number_create(const uint32_t *x, const size_t len,
		const bool minus)
{
	number_t *p = (number_t *) malloc(sizeof(*p));
	if (p) {
		size_t max = ((len + NUMBER_MIN_UNITS - 1) / NUMBER_MIN_UNITS)
				* NUMBER_MIN_UNITS;
		uint32_t *buf = (uint32_t *) malloc(max * sizeof(*buf));
		if (!buf) {
			free(p);
			return NULL;
		}

		int last = 0;
		int i;
		for (i = 0; i < len; i++) {
			uint32_t v = x[i];
			buf[len - i - 1] = v;
			if (!last && v)
				last = len - i - 1;
		}

		for (; i < max; i++)
			buf[i] = 0;

		p->minus = minus;
		p->max = max;
		p->buf = buf;
		p->len = last + 1;
	}

	return p;
}

static void number_destroy(number_t *p)
{
	if (p) {
		free(p->buf);
		free(p);
	}
}

static int number_multiply(number_t *p, const uint32_t n)
{
	if (!p)
		return -1;

	size_t max;
	uint32_t *buf;
	size_t len = p->len + 1;
	if (p->max < len) {
		max = ((len + NUMBER_MIN_UNITS - 1) / NUMBER_MIN_UNITS)
				* NUMBER_MIN_UNITS;
		buf = (uint32_t *) malloc(max * sizeof(uint32_t));
		if (!buf)
			return -1;

		for (int i = len; i < max; i++)
			buf[i] = 0;
	} else {
		max = p->max;
		buf = p->buf;
	}

	int last = 0;
	uint32_t carry = 0;
	for (int i = 0; i < len; i++) {
		uint64_t v = (i < p->len) ? ((uint64_t) (p->buf[i]) * n) : 0;
		v += carry;
		carry = (v >> NUMBER_CARRY_BIT) & NUMBER_VALUE_MASK;
		buf[i] = v & NUMBER_VALUE_MASK;

		if (v)
			last = i;
	}

	p->len = last + 1;

	if (p->max < len) {
		free(p->buf);
		p->max = max;
		p->buf = buf;
	}

	return 0;
}

static int number_shift(number_t *p, const int n)
{
	if (!p || n < 0)
		return -1;

	if (!n)
		return 0;

	size_t len = p->len
			+ ((abs(n) + (NUMBER_CARRY_BIT - 1)) / NUMBER_CARRY_BIT)
			+ 1;
	if (p->max < len) {
		size_t max = ((len + NUMBER_MIN_UNITS - 1) / NUMBER_MIN_UNITS)
				* NUMBER_MIN_UNITS;
		uint32_t *buf = (uint32_t *) malloc(max * sizeof(uint32_t));
		if (!buf)
			return -1;

		int i;
		for (i = 0; i < len; i++)
			buf[i] = p->buf[i];

		for (; i < max; i++)
			buf[i] = 0;

		free(p->buf);
		p->max = max;
		p->buf = buf;
	}

	int last = 0;
	uint32_t carry = 0;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < len; j++) {
			uint32_t v = (p->buf[j] << 1) | carry;
			carry = (v >> NUMBER_CARRY_BIT) & 1;
			p->buf[j] = v & NUMBER_VALUE_MASK;

			if (v)
				last = j;
		}

	p->len = last + 1;
	return 0;
}

static int number_divide(number_t *p, const uint32_t n)
{
	if (!p || !n)
		return -1;

	if (n == 1)
		return 0;

	int last = 0;
	uint32_t reminder = 0;
	for (int i = p->len; i > 0; i--) {
		uint32_t x = (reminder << ((INT_BIT / 2) - 1))
				| (p->buf[i - 1] >> (INT_BIT / 2));
		ldiv_t v = ldiv(x, n);
		uint32_t q = v.quot << (INT_BIT / 2);
		x = (v.rem << (INT_BIT / 2))
				| (p->buf[i - 1]
				& ((1 << (INT_BIT / 2)) - 1));
		v = ldiv(x, n);
		reminder = v.rem;
		q |= v.quot;
		p->buf[i - 1] = q;

		if (!last && q)
			last = i - 1;
	}

	p->len = last + 1;
	return reminder;
}

static void putdouble(State *s, const bool minus, uint64_t sig, const int exp)
{
	uint64_t x = 1;
	int i;
	for (i = 0; sig; i++) {
		x = (x << 1) | (sig >> (B64_SIGNIFICANT_BITS - 1));
		sig = (sig << 1) & B64_SIGNIFICANT_MASK;
	}

	uint32_t v[2];
	v[0] = (uint32_t) (x >> NUMBER_CARRY_BIT);
	v[1] = (uint32_t) (x & NUMBER_VALUE_MASK);

	number_t *num = number_create(v, sizeof(v) / sizeof(v[0]), false);
	if (!num)
		return;

	number_shift(num, exp);

	if (exp < 0)
		i -= exp;

	int dot = i;
	for (; i > NUM_OF_POWER5; i -= NUM_OF_POWER5)
		number_multiply(num, power5[NUM_OF_POWER5 - 1]);

	if (i > 0)
		number_multiply(num, power5[i - 1]);

	unsigned char buf[B64_EXPONENT_MAX_R10 + 3];
	memset(buf, '\0', sizeof(buf));

	int figures = 0;
	for (i = sizeof(buf) - 2; num->len > 1 || num->buf[0]; i--) {
		buf[i] = number_divide(num, 10) + '0';
		figures++;
	}

	dot = figures - dot;

	unsigned char *p = &(buf[i + 1]);
#if 0
	//TODO fix round
	if (p[B64_SIGNIFICANT_FIGURES] >= '5') {
		bool carry = true;

		for (i = B64_SIGNIFICANT_FIGURES - 1; i >= 0; i--)
			if (carry) {
				unsigned char c = p[i];
				carry = (c == '9');
				p[i] = carry ? '0' : (c + 1);
			}
	}
#endif
	if (dot <= 0) {
		_puts(s, "0.");

		for (; dot < 0; dot++)
			_putchar(s, '0');

		dot = -1;
	}

	for (i = 0; i < B64_SIGNIFICANT_FIGURES; i++) {
		if (i == dot)
			_putchar(s, '.');

		if (*p) {
			_putchar(s, *p);
			p++;
		} else
			_putchar(s, '0');
	}

	number_destroy(num);
}

static void _putf(State *s, const double x)
{
	unsigned char *p = (unsigned char *) &x;
	int exp = ((p[7] << 4) & 0x7f0) | ((p[6] >> 4) & 0x00f);
	uint64_t sig = (p[6] << 16) & 0x000f0000;

	sig |= (p[5] << 8) & 0x0000ff00;
	sig |= p[4] & 0x000000ff;
	sig <<= 32;

	sig |= (p[3] << 24) & 0xff000000;
	sig |= (p[2] << 16) & 0x00ff0000;
	sig |= (p[1] << 8) & 0x0000ff00;
	sig |= p[0] & 0x000000ff;

	if (sig && (exp == B64_EXPONENT_SPECIAL)) {
		_puts(s, "NaN");
		return;
	}

	bool minus = (p[7] & 0x80) ? true : false;
	if (minus)
		_putchar(s, '-');

	if (exp == B64_EXPONENT_SPECIAL)
		_puts(s, "oo");
	else if (!exp && !sig)
		_puts(s, "0.0");
	else
		putdouble(s, minus, sig, exp - B64_EXPONENT_BIAS);
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
		_putchar(s, va_arg(s->ap, int) & 0xff);
		break;
	case 'd':
		_putd(s, va_arg(s->ap, int), 10);
		break;
#ifdef USE_FLOAT
	case 'f':
		_putf(s, va_arg(s->ap, double));
		break;
#endif
	case 'l': {
			char c2 = *(s->format)++;
			switch (c2) {
			case 'd':
				_putd(s, va_arg(s->ap, long), 10);
				break;
			case 'o':
				_putd(s, va_arg(s->ap, long), 8);
				break;
			case 'x':
				_puth(s, va_arg(s->ap, long));
				break;
			default:
				_putchar(s, '%');
				_putchar(s, c);
				_putchar(s, c2);
				break;
			}
		}
		break;
	case 'o':
		_putd(s, va_arg(s->ap, int), 8);
		break;
	case 'p':
		_putchar(s, '0');
		_putchar(s, 'x');
	case 'x':
		_puth(s, va_arg(s->ap, int));
		break;
	case 's':
		_puts(s, va_arg(s->ap, char *));
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

int vnprintf2(int (*out)(const char, void *), void *env,
		const char *format, va_list ap) {
	State s = { _immediate, format, ap, out, env, 0 };
	while (s.handler(&s));

	return s.len;
}
