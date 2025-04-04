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
#include <mpu/limits.h>
#include "funcs.h"

#if math_errhandling & MATH_ERRNO
#include <errno.h>
#endif


double exp(double x)
{
	if (!x)
		return 1.0;

	int *p = (int *) &x;
	if (((p[1] >> EXP_SHIFT_U) & EXP_MAX) == EXP_MAX)
		return ((x == -INFINITY) ? (+0.0) : x);

	int k = (int) (x / M_LN2 + ((x >= 0) ? 0.5 : (-0.5)));
	x -= k * M_LN2;

	double result;
	double x2 = x * x;
	if (isinf(x2))
		result = x2;
	else {
		double sum = x2 / 22;
		sum = x2 / (sum + 18);
		sum = x2 / (sum + 14);
		sum = x2 / (sum + 10);
		sum = x2 / (sum + 6);
		sum += 2;
		result = ldexp((sum + x) / (sum - x), k);
	}
#if math_errhandling & MATH_ERRNO
	if (isinf(result))
		// overflow
		errno = ERANGE;
	else
#endif
	if (!result) {
		// underflow
#if math_errhandling & MATH_ERRNO
		errno = ERANGE;
#endif
		result = +0.0;
	}

	return result;
}
