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
#include <mpu/ieee754.h>

#define LOG_2 0.6931471805599453094172321214581765680755L
#define SQRT_2 1.4142135623730950488016887242096980785696L

#define INT_BITS (CHAR_BIT * sizeof(int))
#define SIGN_MASK_U (1 << (INT_BITS - 1))
#define EXP_MAX B64_EXPONENT_SPECIAL
#define EXP_SHIFT_U (B64_SIGNIFICANT_BITS - INT_BITS)


double log(double x)
{
	int *p = (int *) &x;
	int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if (e == 0) {
		errno = ERANGE;
		return (-HUGE_VAL);
	}

	if (p[1] & SIGN_MASK_U) {
		errno = EDOM;
		return NAN;
	}

	if (e == EXP_MAX)
		return x;

	if (x == 1.0)
		return (+0.0);

	int k;
	frexp(x / SQRT_2, &k);
	x /= ldexp(1, k);
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

	return (LOG_2 * k + 2 * sum);
}
