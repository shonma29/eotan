#ifndef __ARCH_SCAN2KEY_H__
#define __ARCH_SCAN2KEY_H__ 1
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

#define SCAN_BREAK 0x80

enum {
 scan_normal = 0,
 scan_e0 = 1,
 scan_e1 = 2
};

static unsigned char scan2key[3][128] = {
	/* base */
	{
		0, 110, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 15, 16,
		17, 18, 19, 20, 21, 22, 23, 24,
		25, 26, 27, 28, 43, 58, 31, 32,
		33, 34, 35, 36, 37, 38, 39, 40,
		41, 0, 44, 42, 46, 47, 48, 49,
		50, 51, 52, 53, 54, 55, 57, 100,
		60, 61, 30, 112, 113, 114, 115, 116,
		117, 118, 119, 120, 121, 90, 125, 91,
		96, 0, 105, 92, 97, 102, 106, 93,
		98, 103, 99, 104, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 122,
		123, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 56, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	},

	/* e0 */
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 64, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 44, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 95, 57, 100,
		62, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 80,
		0, 0, 0, 79, 0, 0, 0, 0,
		0, 0, 75, 76, 0, 0, 0, 0,
		83, 85, 0, 0, 0, 89, 0, 81,
		84, 86, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	},

	/* e1 */
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 58, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	}
};

static inline unsigned char is_break(const unsigned char b)
{
	return SCAN_BREAK & b;
}

static inline unsigned char strip_break(const unsigned char b)
{
	return (~SCAN_BREAK) & b;
}

#endif
