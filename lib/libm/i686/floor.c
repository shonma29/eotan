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
#include <stdint.h>
#include <stdbool.h>
#include <mpu/ieee754.h>

#define INT_BITS (CHAR_BIT * sizeof(int))
#define EXP_MAX B64_EXPONENT_SPECIAL
#define EXP_SHIFT_U (B64_SIGNIFICANT_BITS - INT_BITS)
#define SIG_MASK_U ((1 << EXP_SHIFT_U) - 1)


double floor(double x)
{
	if (!x)
		return x;

	uint32_t *p = (uint32_t *) &x;
	int exp = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
	if (exp == B64_EXPONENT_SPECIAL)
		return x;

	bool minus = ((int32_t) p[1]) < 0;
	exp -= B64_EXPONENT_BIAS;
	if (exp <  0)
		return (minus ? (-1) : 0);

	if (exp >= B64_SIGNIFICANT_BITS)
		return x;

	double result;
	uint32_t *q = (uint32_t *) &result;
	if (exp <= EXP_SHIFT_U) {
		uint32_t mask1 = (1 << (EXP_SHIFT_U - exp)) - 1;
		q[0] = 0;
		q[1] = p[1] & ~mask1;

		if (minus
				&& (p[0] || (p[1] & mask1)))
			result -= 1;
	} else {
		uint32_t mask0 = ((1 << (B64_SIGNIFICANT_BITS - exp)) - 1);
		q[0] = p[0] & ~mask0;
		q[1] = p[1];

		if (minus
				&& (p[0] & mask0))
			result -= 1;
	}

	return result;
}
