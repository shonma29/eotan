#ifndef __HMI_KEY2CHAR_H__
#define __HMI_KEY2CHAR_H__ 1
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
enum {
	key_base = 0,
	key_shift = 1,
	key_ctrl = 2
};

static unsigned char key2char[3][128] = {
	/* base */
	{
		0, '`', '1', '2', '3', '4', '5', '6',
		'7', '8', '9', '0', '-', '=', 0, 8,
		9, 'q', 'w', 'e', 'r', 't', 'y', 'u',
		'i', 'o', 'p', '[', ']', 0, 0, 'a',
		's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
		';', '\'', '\\', 10, 0, 0, 'z', 'x',
		'c', 'v', 'b', 'n', 'm', ',', '.', '/',
		0, 0, 0, 0, 0, ' ', 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, '7', '4', '1', 0, '/',
		'8', '5', '2', '0', '*', '9', '6', '3',
		'.', '-', '+', 0, 10, 0, 27, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	},

	/* shift */
	{
		0, '~', '!', '@', '#', '$', '%', '^',
		'&', '*', '(', ')', '_', '+', 0, 8,
		9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
		'I', 'O', 'P', '{', '}', 0, 0, 'A',
		'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
		':', '\"', '|', 10, 0, 0, 'Z', 'X',
		'C', 'V', 'B', 'N', 'M', '<', '>', '?',
		0, 0, 0, 0, 0, ' ', 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, '7', '4', '1', 0, '/',
		'8', '5', '2', '0', '*', '9', '6', '3',
		'.', '-', '+', 0, 10, 0, 27, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	},

	/* cntl */
	{
		255, 255, 255, 0, 255, 255, 255, 30,
		255, 255, 255, 255, 31, 255, 255, 255,
		255, 17, 23, 5, 18, 20, 25, 21,
		9, 15, 16, 27, 29, 255, 255, 1,
		19, 4, 6, 7, 8, 10, 11, 12,
		255, 255, 28, 255, 255, 255, 26, 24,
		3, 22, 2, 14, 13, 255, 255, 127,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 4, 255, 255, 2,
		1, 5, 255, 16, 14, 26, 22, 255,
		255, 6, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255
	}
};

#endif
