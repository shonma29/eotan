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
#include <mpu/bits.h>
#include <mpu/ieee754.h>
#include "funcs.h"

#define EXP_BIAS_ZERO (B64_EXPONENT_BIAS - 1)


double frexp(double num, int *exp)
{
	if (num) {
		int *p = (int *) &num;
		int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
		if (!e) {
			int shift = count_nlz(p[1] & SIG_MASK_U);
			if (shift == INT_BITS) {
				shift = count_nlz(p[0]);
				if (shift == INT_BITS)
					return num;
			} else
				shift += INT_BITS;

			*exp = e - EXP_BIAS_ZERO
					- (B64_SIGNIFICANT_BITS - 1 - shift);
			p[0] = 0;
			p[1] = (p[1] & SIGN_MASK_U)
					| (EXP_BIAS_ZERO << EXP_SHIFT_U);
		} else if (e != EXP_MAX) {
			*exp = e - EXP_BIAS_ZERO;
			p[1] = (p[1] & ~EXP_MASK_U)
					| (EXP_BIAS_ZERO << EXP_SHIFT_U);
		}
	} else
		*exp = 0;

	return num;
}
