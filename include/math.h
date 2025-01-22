#ifndef _MATH_H_
#define _MATH_H_
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

#define MATH_ERRNO (1)
#define MATH_ERREXCEPT (2)
#define math_errhandling (MATH_ERRNO)

#define NAN (__builtin_nan(""))
#define INFINITY (__builtin_inf())
#define HUGE_VAL (__builtin_huge_val())

#define M_PI (3.1415926535897932384626433832795028841971L)
#define M_LN2 (0.6931471805599453094172321214581765680755L)
#define M_SQRT2 (1.4142135623730950488016887242096980785696L)

extern int __isinf_double(const double);
extern int __isnan_double(const double);

#define isinf(x) _Generic((x), \
	double: __isinf_double \
)(x)
#define isnan(x) _Generic((x), \
	double: __isnan_double \
)(x)

extern double fabs(double);
extern double frexp(double, int *);
extern double ldexp(double, int);

extern double exp(double);
extern double log(double);
extern double sqrt(double);

extern double sin(double);
extern double cos(double);
extern double tan(double);
extern double atan(double);

extern double floor(double);

#endif
