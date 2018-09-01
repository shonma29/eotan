#ifndef _MPU_BITS_H_
#define _MPU_BITS_H_
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

static inline int count_bits(int d)
{
	d -= ((d >> 1) & 0x55555555);
	d = (d & 0x33333333) + ((d >> 2) & 0x33333333);
	d = (d + (d >> 4)) & 0x0f0f0f0f;

	return (d * 0x01010101) >> 24;
}

static inline int count_ntz(int d)
{
	return count_bits((d & (-d)) - 1);
}

static inline int count_nlz(int d)
{
	d |= d >> 1;
	d |= d >> 2;
	d |= d >> 4;
	d |= d >> 8;
	d |= d >> 16;

	return count_bits(~d);
}

#endif
