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
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <mpu/ieee754.h>
#include "funcs.h"

#define DEGREE (17)
#define RESIDUAL (4.4544551033807686783083602485579e-6)

typedef struct {
	double r;
	int k;
} tan_t;

static void _tan(tan_t *t)
{
	t->k = (int) (t->r / (M_PI / 2) + ((t->r >= 0) ? 0.5 : -0.5));

	double r = (t->r - (3217.0 / 2048) * t->k) + RESIDUAL * t->k;
	double r2 = r * r;
	double v = 0;
	for (int i = DEGREE; i >= 3; i -= 2)
		v = r2 / (i - v);

	t->r = r / (1 - v);;
}

double tan(const double x)
{
	if (x == 0.0)
		return x;

	int *p = (int *) &x;
	int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if (e == EXP_MAX) {
		if (!p[0]
				&& !(p[1] & SIG_MASK_U))
			errno = EDOM;

		return NAN;
	}

	tan_t t;
	t.r = x;
	_tan(&t);
	if (t.r == 0.0)
		t.r = (x < 0) ? (-0.0) : 0.0;

	return ((t.k & 1) ? (t.r ? ((-1) / t.r) : HUGE_VAL) : t.r);
}

double sin(const double x)
{
	if (x == 0.0)
		return x;

	int *p = (int *) &x;
	int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if (e == EXP_MAX) {
		if (!p[0]
				&& !(p[1] & SIG_MASK_U))
			errno = EDOM;

		return NAN;
	}

	tan_t t;
	t.r = x / 2;
	_tan(&t);

	t.r = (t.r + t.r) / (1 + t.r * t.r);
	if (t.r == 0.0)
		t.r = (x < 0) ? (-0.0) : 0.0;

	return ((t.k & 1) ? (-t.r) : t.r);
}

double cos(const double x)
{
	return sin(M_PI / 2 - fabs(x));
}
