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
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <mpu/ieee754.h>
#include "funcs.h"

#define EXTRA_BIT_U (1 << EXP_SHIFT_U)


double ldexp(double x, int exp)
{
	do {
		if (!exp || !x)
			break;

		int *p = (int *) &x;
		int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
		if (e == EXP_MAX)
			break;

		if (exp > 0) {
			if (exp >= (EXP_MAX - e)) {
				errno = ERANGE;
				x = ((p[1] & SIGN_MASK_U) ?
						-HUGE_VAL : HUGE_VAL);
				break;
			}

		} else if (exp <= (-e - B64_SIGNIFICANT_BITS)) {
			errno = ERANGE;
			x = 0;
			break;
		}

		e += exp;
		if (e <= 0) {
			int v;
			int w = (p[1] & SIG_MASK_U) + EXTRA_BIT_U;
			int shift = 1 - e;
			if (shift < INT_BITS) {
				v = p[0] >> shift;
				v &= (1 << (INT_BITS - shift)) - 1;
				v |= w << (INT_BITS - shift);
				w >>= shift;
			} else {
				v = w >> (shift - INT_BITS);
				if (shift != B64_SIGNIFICANT_BITS)
					v++;

				w = 0;
			}

			p[0] = v;
			p[1] = (p[1] & SIGN_MASK_U) | w;
		} else
			p[1] = (p[1] & ~EXP_MASK_U) | (e << EXP_SHIFT_U);
	} while (false);

	return x;
}
