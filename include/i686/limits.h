#ifndef _MPU_LIMITS_H_
#define _MPU_LIMITS_H_
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

#define CHAR_BIT (__CHAR_BIT__)

#define MB_LEN_MAX 6

#define SCHAR_MAX (__SCHAR_MAX__)
#define SCHAR_MIN (-SCHAR_MAX - 1)
#define UCHAR_MAX ((unsigned char) SCHAR_MAX * 2 + 1)
#define CHAR_MAX SCHAR_MAX
#define CHAR_MIN SCHAR_MIN

#define SHRT_MAX (__SHRT_MAX__)
#define SHRT_MIN (-SHRT_MAX - 1)
#define USHRT_MAX ((unsigned short) SHRT_MAX * 2 + 1)

#define INT_MAX (__INT_MAX__)
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX ((unsigned int) INT_MAX * 2 + + 1)

#define LONG_MAX (__LONG_MAX__)
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX ((unsigned long) LONG_MAX * 2 + 1)

#define LLONG_MAX (__LONG_LONG_MAX__)
#define LLONG_MIN (-LLONG_MAX - 1)
#define ULLONG_MAX ((unsigned long long) LLONG_MAX * 2 + 1)

#endif
