#ifndef _MPU_STDINT_H_
#define _MPU_STDINT_H_
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
#include <mpu/limits.h>

typedef char int8_t;
typedef unsigned char uint8_t;
typedef char int_least8_t;
typedef unsigned char uint_least8_t;
typedef int int_fast8_t;
typedef unsigned int uint_fast8_t;

typedef short int16_t;
typedef unsigned short uint16_t;
typedef short int_least16_t;
typedef unsigned short uint_least16_t;
typedef int int_fast16_t;
typedef unsigned int uint_fast16_t;

typedef long int int32_t;
typedef unsigned long int uint32_t;
typedef long int int_least32_t;
typedef unsigned long int uint_least32_t;
typedef long int int_fast32_t;
typedef unsigned long int uint_fast32_t;

typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef long long int int_least64_t;
typedef unsigned long long int uint_least64_t;
typedef long long int int_fast64_t;
typedef unsigned long long int uint_fast64_t;

typedef long int intptr_t;
typedef unsigned long int uintptr_t;
typedef long long int intmax_t;
typedef unsigned long long int uintmax_t;

#define INT8_MIN CHAR_MIN
#define INT8_MAX CHAR_MAX
#define UINT8_MAX UCHAR_MAX
#define INT_LEAST8_MIN CHAR_MIN
#define INT_LEAST8_MAX CHAR_MAX
#define UINT_LEAST8_MAX UCHAR_MAX
#define INT_FAST8_MIN INT_MIN
#define INT_FAST8_MAX INT_MAX
#define UINT_FAST8_MAX UINT_MAX

#define INT16_MIN SHRT_MIN
#define INT16_MAX SHRT_MAX
#define UINT16_MAX USHRT_MAX
#define INT_LEAST16_MIN SHRT_MIN
#define INT_LEAST16_MAX SHRT_MAX
#define UINT_LEAST16_MAX USHRT_MAX
#define INT_FAST16_MIN INT_MIN
#define INT_FAST16_MAX INT_MAX
#define UINT_FAST16_MAX UINT_MAX

#define INT32_MIN LONG_MIN
#define INT32_MAX LONG_MAX
#define UINT32_MAX ULONG_MAX
#define INT_LEAST32_MIN LONG_MIN
#define INT_LEAST32_MAX LONG_MAX
#define UINT_LEAST32_MAX ULONG_MAX
#define INT_FAST32_MIN LONG_MIN
#define INT_FAST32_MAX LONG_MAX
#define UINT_FAST32_MAX ULONG_MAX

#define INT64_MIN LLONG_MIN
#define INT64_MAX LLONG_MAX
#define UINT64_MAX ULLONG_MAX
#define INT_LEAST64_MIN LLONG_MIN
#define INT_LEAST64_MAX LLONG_MAX
#define UINT_LEAST64_MAX ULLONG_MAX
#define INT_FAST64_MIN LLONG_MIN
#define INT_FAST64_MAX LLONG_MAX
#define UINT_FAST64_MAX ULLONG_MAX

#define INTPTR_MIN LONG_MIN
#define INTPTR_MAX LONG_MAX
#define UINTPTR_MAX ULONG_MAX

#define INTMAX_MIN LLONG_MIN
#define INTMAX_MAX LLONG_MAX
#define UINTMAX_MAX ULLONG_MAX

#define INT8_C(v) v
#define UINT8_C(v) v
#define INT16_C(v) v
#define UINT16_C(v) v
#define INT32_C(v) v ## L
#define UINT32_C(v) v ## UL
#define INT64_C(v) v ## LL
#define UINT64_C(v) v ## ULL
#define INTMAX_C(v) v ## LL
#define UINTMAX_C(v) v ## ULL

#endif
