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
#include <math.h>
#include <mpu/ieee754.h>
#include "funcs.h"

#define DEGREE (21)


double atan(double x)
{
	if (x == 0.0)
		return x;

	int *p = (int *) &x;
	int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if (e == EXP_MAX) {
		if (p[0]
				|| (p[1] & SIG_MASK_U))
			return NAN;

		int sign_bit = p[1] & SIGN_MASK_U;
		x = M_PI / 2;
		p[1] |= sign_bit;
		return x;
	}

	int sign;
	if (x > 1) {
		sign = 1;
		x = 1 / x;
	} else if (x < -1) {
		sign = -1;
		x = 1 / x;
	} else
		sign = 0;

	double sum = 0;
	for (int i = DEGREE; i > 0; i--)
		sum = (i * i * x * x) / (2 * i + 1 + sum);

	sum = x / (1 + sum);

	if (!sign)
		return sum;
	else if (sign > 0)
		return (M_PI / 2 - sum);
	else
		return (-M_PI / 2 - sum);
}
