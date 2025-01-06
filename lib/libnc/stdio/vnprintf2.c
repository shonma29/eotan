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

#ifdef USE_FLOAT
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mpu/ieee754.h>
#endif

#define MAX_INT_COLUMN (10)
#define INT_BIT ((CHAR_BIT) * sizeof(int))
#ifdef _LP64
#define MAX_LONG_COLUMN (20)
#define LONG_BIT ((CHAR_BIT) * sizeof(long))
#endif

#ifdef USE_FLOAT
#define NUMBER_MIN_UNITS (4)
#define NUMBER_CARRY_BIT (INT_BIT)
#define NUMBER_VALUE_MASK 0xffffffff
#define NUMBER_ARRAY_INITIAL_LEN (sizeof(uint64_t) / sizeof(uint32_t))
#define NUMBER_ARRAY_MAX (((NUMBER_ARRAY_INITIAL_LEN + NUMBER_MIN_UNITS - 1) \
		/ NUMBER_MIN_UNITS) \
		* NUMBER_MIN_UNITS)

#define B64_SIGNIFICANT_MASK (((uint64_t) 1 << B64_SIGNIFICANT_BITS) - 1)

typedef struct {
	bool minus;
	int len;
	int max;
	uint32_t *buf;
} number_t;
#endif

typedef struct _State {
	bool (*handler)(struct _State *);
	const char *format;
	va_list *ap;
	int (*out)(const char, void *);
	void *env;
	int len;
	bool has_error;
} State;

static void _putchar(State *, const char);
static void _puts(State *, const char *);
static void _putd(State *, const int, const int);
#ifdef _LP64
static void _putld(State *, const long, const int);
#endif
static void _putx(State *, const unsigned int);
#ifdef _LP64
static void _putlx(State *, const unsigned long);
#endif

#ifdef USE_FLOAT
static uint32_t power5[] = {
	5, 25, 125, 625, 3125, 15625, 78125, 390625,
	1953125, 9765625, 48828125, 244140625, 1220703125
};
#define NUM_OF_POWER5 (sizeof(power5) / sizeof(power5[0]))

static number_t *number_create(const uint64_t *, const bool);
static void number_destroy(number_t *);
static int number_multiply(number_t *, const uint32_t);
static int number_shift(number_t *, const int);
static int number_divide_10(number_t *);
static int _number_to_decimal(char *, number_t *, const int);
static int putdouble(State *, const bool, uint64_t, int);
static int _putf(State *, const double);
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
	for (char c; (c = *str); str++)
		_putchar(s, c);
}

static void _putd(State *s, const int v, const int radix)
{
	unsigned int u;
	if ((radix == 10) && v < 0) {
		_putchar(s, '-');
		u = -v;
	} else
		u = v;

	char buf[MAX_INT_COLUMN + 1];
	char *p = &buf[sizeof(buf) - 1];
	*p = '\0';

	do {
		*--p = (u % radix) + '0';
	} while (u /= radix);

	_puts(s, p);
}
#ifdef _LP64
static void _putld(State *s, const long v, const int radix)
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
		*--p = (u % radix) + '0';
	} while (u /= radix);

	_puts(s, p);
}
#endif
static void _putx(State *s, const unsigned int u)
{
	int shift = count_nlz(u);
	if (shift < INT_BIT) {
		shift &= INT_BIT - 1 - 3;
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
		shift &= LONG_BIT - 1 - 3;
		do {
			int c = (u >> shift) & 0xf;
			_putchar(s, c + ((c >= 10) ? ('a' - 10) : '0'));
		} while ((shift -= 4) >= 0);
	} else
		_putchar(s, '0');
}
#endif
#ifdef USE_FLOAT
static number_t *number_create(const uint64_t *x, const bool minus)
{
	number_t *p = (number_t *) malloc(sizeof(*p));
	if (p) {
		uint32_t *buf = (uint32_t *) malloc(NUMBER_ARRAY_MAX * sizeof(*buf));
		if (!buf) {
			free(p);
			return NULL;
		}

		uint32_t *int_ptr = (uint32_t *) x;
		int last = 0;
		int i;
		for (i = 0; i < NUMBER_ARRAY_INITIAL_LEN; i++) {
			uint32_t v = int_ptr[i];
			buf[i] = v;
			if (v)
				last = i;
		}

		for (; i < NUMBER_ARRAY_MAX; i++)
			buf[i] = 0;

		p->minus = minus;
		p->max = NUMBER_ARRAY_MAX;
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

	int max;
	uint32_t *buf;
	int len = p->len + 1;
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
		uint64_t v = (i < p->len) ? (((uint64_t) p->buf[i]) * n) : 0;
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

	int first_block = -1;
	for (int i = 0; i < p->len; i++)
		if (p->buf[i]) {
			first_block = i;
			break;
		}

	if (first_block < 0)
		return 0;

	int len = p->len + ((n + (INT_BIT - 1)) / INT_BIT);
	if (p->max < len) {
		int max = (len + NUMBER_MIN_UNITS - 1) / NUMBER_MIN_UNITS
				* NUMBER_MIN_UNITS;
		uint32_t *buf = (uint32_t *) malloc(max * sizeof(*buf));
		if (!buf)
			return -1;

		int i;
		for (i = 0; i < p->len; i++)
			buf[i] = p->buf[i];

		for (; i < max; i++)
			buf[i] = 0;

		free(p->buf);
		p->max = max;
		p->buf = buf;
	}

	len = p->len;

	int q = n / INT_BIT;
	int r = n - q * INT_BIT;
	if (r == 0) {
		int i = len - 1;
		do {
			p->buf[i + q] = p->buf[i];
		} while (--i >= first_block);

		p->len += q;
	} else {
		uint32_t rest = 0;
		int i = len - 1;
		do {
			p->buf[i + q + 1] = rest | (p->buf[i] >> (INT_BIT - r));
			rest = p->buf[i] << r;
		} while (--i >= first_block);

		p->buf[i + q + 1] = rest;
		p->len = p->buf[len + q] ? (len + q + 1) : (len + q);
	}

	len = ((first_block + q) < len) ? (first_block + q) : len;
	for (int i = first_block; i < len; i++)
		p->buf[i] = 0;

	return 0;
}

static int number_divide_10(number_t *p)
{
	if (!p)
		return -1;

	int last = 0;
	uint32_t reminder = 0;
	for (int i = p->len; i > 0; i--) {
		uint32_t x = (reminder << ((INT_BIT / 2)))
				| (p->buf[i - 1] >> (INT_BIT / 2));
		ldiv_t v = ldiv(x, 10);
		uint32_t q = v.quot << (INT_BIT / 2);
		x = (v.rem << (INT_BIT / 2))
				| (p->buf[i - 1]
				& ((1 << (INT_BIT / 2)) - 1));
		v = ldiv(x, 10);
		reminder = v.rem;
		q |= v.quot;
		p->buf[i - 1] = q;

		if (!last && q)
			last = i - 1;
	}

	p->len = last + 1;
	return reminder;
}

static int _number_to_decimal(char *p, number_t *num, const int max)
{
	int i = max - 1;
	for (p[i] = '\0'; num->len > 1 || num->buf[0];)
		p[--i] = number_divide_10(num) + '0';

	return i;
}

static int putdouble(State *s, const bool minus, uint64_t sig, int exp)
{
	uint64_t x;
	int decimal_shift;
	if (exp == -B64_EXPONENT_BIAS) {
		x = sig;
		decimal_shift = B64_SIGNIFICANT_BITS + B64_EXPONENT_BIAS - 1;
	} else {
		//TODO use count_nlz
		int i;
		x = 1;
		for (i = 0; sig; i++) {
			x = (x << 1) | (sig >> (B64_SIGNIFICANT_BITS - 1));
			sig = (sig << 1) & B64_SIGNIFICANT_MASK;
		}

		decimal_shift = i - exp;
	}

	number_t *num = number_create((uint64_t *) &x, false);
	if (!num) {
		errno = ENOMEM;
		s->has_error = true;
		return (-1);
	}

	if (decimal_shift > 0) {
		int i = decimal_shift;
		for (; i >= (int) NUM_OF_POWER5; i -= NUM_OF_POWER5)
			number_multiply(num, power5[NUM_OF_POWER5 - 1]);

		if (i > 0)
			number_multiply(num, power5[i - 1]);
	} else {
		number_shift(num, - decimal_shift);
		decimal_shift = 0;
	}

	char buf[B64_EXPONENT_MAX_R10 + 1];
	int head = _number_to_decimal(buf, num, sizeof(buf));
	int integer_len = sizeof(buf) - 1 - head - decimal_shift;
	if (integer_len <= 0) {
		_puts(s, "0.");

		for (int i = 0; i < - integer_len; i++)
			_putchar(s, '0');
	} else {
		for (int i = 0; i < integer_len; i++)
			_putchar(s, buf[head + i]);

		_putchar(s, '.');
		head += integer_len;

		if (!decimal_shift)
			_putchar(s, '0');
	}

	_puts(s, &(buf[head]));
	number_destroy(num);
	return 0;
}

static int _putf(State *s, const double x)
{
	uint64_t *long_ptr = (uint64_t *) &x;
	uint64_t sig = *long_ptr & B64_SIGNIFICANT_MASK;
	int *int_ptr = (int *) &x;
	int exp = (int_ptr[1] >> (INT_BIT - 1 - B64_EXPONENT_BITS))
			& B64_EXPONENT_SPECIAL;
	if (sig && (exp == B64_EXPONENT_SPECIAL)) {
		_puts(s, "nan");
		return 0;
	}

	bool minus = int_ptr[1] < 0;
	if (minus)
		_putchar(s, '-');

	if (exp == B64_EXPONENT_SPECIAL)
		_puts(s, "inf");
	else if (!exp && !sig)
		_puts(s, "0.0");
	else
		return putdouble(s, minus, sig, exp - B64_EXPONENT_BIAS);

	return 0;
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
		_putd(s, va_arg(*(s->ap), int), 10);
		break;
#ifdef USE_FLOAT
	case 'f':
		if (_putf(s, va_arg(*(s->ap), double)))
			return false;

		break;
#endif
	case 'l': {
			char c2 = *(s->format)++;
			switch (c2) {
			case 'd':
#ifdef _LP64
				_putld(s, va_arg(*(s->ap), long), 10);
#else
				_putd(s, va_arg(*(s->ap), long), 10);
#endif
				break;
			case 'o':
#ifdef _LP64
				_putld(s, va_arg(*(s->ap), long), 8);
#else
				_putd(s, va_arg(*(s->ap), long), 8);
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
	case 'o':
		_putd(s, va_arg(*(s->ap), int), 8);
		break;
	case 'p':
		_putchar(s, '0');
		_putchar(s, 'x');
#ifdef _LP64
		_putlx(s, va_arg(*(s->ap), uintptr_t));
#else
		_putx(s, va_arg(*(s->ap), uintptr_t));
#endif
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

int vnprintf2(int (*out)(const char, void *), void *env,
		const char *format, va_list *ap) {
	State s = { _immediate, format, ap, out, env, 0, false };
	while (s.handler(&s));

	return s.has_error ? (-1) : s.len;
}
