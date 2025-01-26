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
#include <mpu/ieee754.h>
#include <mpu/bits.h>
#include "funcs.h"

#define EXTRA_BIT_U (1 << EXP_SHIFT_U)

static int _count_sig_nlz(const double *);
static int _count_sig_ntz(const double *);

/**
 * Count leading zeros of denormalized number.
 * @param p pointer to 'double' value
 * @return number of leading zero
 */
static int _count_sig_nlz(const double *ptr_to_x)
{
	uint32_t *p = (uint32_t *) ptr_to_x;
	int nlz = count_nlz(p[1]);
	return ((nlz == INT_BITS) ? count_nlz(p[0]) : (INT_BITS + nlz));
}

/**
 * Count trailing zeros of denormalized number.
 * @param p pointer to 'double' value
 * @return number of leading zero
 */
static int _count_sig_ntz(const double *ptr_to_x)
{
	uint32_t *p = (uint32_t *) ptr_to_x;
	int ntz = count_ntz(p[0]);
	return ((ntz == INT_BITS) ? (INT_BITS + count_ntz(p[1])) : ntz);
}

double ldexp(double x, int exp)
{
	do {
		if (!exp || !x)
			break;

		uint64_t *q = (uint64_t *) &x;
		uint32_t *p = (uint32_t *) &x;
		int e = (p[1] >> EXP_SHIFT_U) & EXP_MAX;
		if (e == EXP_MAX)
			break;

		int nlz = 0;
		uint32_t sign = p[1] & SIGN_MASK_U;
		p[1] &= SIG_MASK_U;

		if (e) {
			if (exp <= -e) {
				// denormalize
				nlz = B64_SIGNIFICANT_BITS - 1;
				exp += e;
				e = 0;
				p[1] |= EXTRA_BIT_U;
				*q >>= 1;
			}
		} else {
			nlz = _count_sig_nlz(&x);
			if ((B64_SIGNIFICANT_BITS - 1 - nlz) < exp) {
				// normalize
				nlz = B64_SIGNIFICANT_BITS - nlz;
				exp -= nlz;
				e = 1;
				*q <<= nlz;
				p[1] &= SIG_MASK_U;
			}
		}

		if (e) {
			if (exp >= (EXP_MAX - e)) {
#if math_errhandling & MATH_ERRNO
				errno = ERANGE;
#endif
				x = HUGE_VAL;
			} else
				p[1] |= (e + exp) << EXP_SHIFT_U;
		} else if (exp >= 0) {
			*q <<= exp;
		} else {
			if (exp < -_count_sig_ntz(&x)) {
#if math_errhandling & MATH_ERRNO
				errno = ERANGE;
#endif
			}

			if (exp < -(nlz + 1))
				x = 0.0;
			else {
				// round
				int t = - exp - 1;
				if (*q & (1 << t)) {
					if (*q & (1 << (t + 1)))
						*q += 1 << t;
					else if (t)
						*q += 1 << (t - 1);
				}

				*q >>= -exp;
			}
		}

		p[1] |= sign;
	} while (false);

	return x;
}
