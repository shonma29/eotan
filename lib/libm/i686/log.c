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
#include <math.h>
#include <mpu/ieee754.h>
#include "funcs.h"

#if math_errhandling & MATH_ERRNO
#include <errno.h>
#endif

double log(double x)
{
	if (x == 0) {
#if math_errhandling & MATH_ERRNO
		errno = ERANGE;
#endif
		return (-HUGE_VAL);
	}

	if (x == 1.0)
		return (+0.0);

	int *p = (int *) &x;
	int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if ((e == EXP_MAX)
			&& (!(p[1] & SIGN_MASK_U)
					|| p[0]
					|| (p[1] & SIG_MASK_U)))
		return x;

	if (p[1] & SIGN_MASK_U) {
#if math_errhandling & MATH_ERRNO
		errno = EDOM;
#endif
		return -NAN;
	}

	int k;
	frexp(x / M_SQRT2, &k);

	int prev_error = errno;
	double b = ldexp(1, k);
	if (isinf(b)) {
		errno = prev_error;
		return (M_LN2 * k);
	}

	x /= b;
	x = (x - 1) / (x + 1);

	double x2 = x * x;
	double sum = x;
	double prev;
	int i = 1;
	do {
		prev = sum;
		x *= x2;
		i += 2;
		sum += x / i;
	} while (prev != sum);

	return (M_LN2 * k + 2 * sum);
}
