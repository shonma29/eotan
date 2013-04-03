#ifndef _IA32_LIMITS_H_
#define _IA32_LIMITS_H_
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

#define CHAR_BIT 8
#define INT_BIT 32
#define MB_LEN_MAX 6

#define CHAR_MAX 0x7f
#define CHAR_MIN (-0x7f - 1)
#define SCHAR_MAX 0x7f
#define SCHAR_MIN (-0x7f - 1)
#define UCHAR_MAX 0xffU

#define SHRT_MAX 0x7fff
#define SHRT_MIN (-0x7fff - 1)
#define USHRT_MAX 0xffffU

#define INT_MAX 0x7fffffff
#define INT_MIN (-0x7fffffff - 1)
#define UINT_MAX 0xffffffffU

#define LONG_MAX 0x7fffffffL
#define LONG_MIN (-0x7fffffffL - 1)
#define ULONG_MAX 0xffffffffUL

#endif
