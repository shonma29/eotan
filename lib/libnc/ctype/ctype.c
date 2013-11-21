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
#include <ctype.h>

/** nil */
#define C_N 0x00

/** upper case alphabet */
#define C_U 0x01

/** lower case alphabet */
#define C_L 0x02

/** alphabet for hex digit */
#define C_X 0x04

/** digit */
#define C_D 0x08

/** punctuation */
#define C_P 0x10

/** control character */
#define C_C 0x20

/** control space  */
#define C_S 0x40

/** white space */
#define C_W 0x80

static unsigned char tbl[257] = {
	/* EOF */
	C_N,

	/* 0x00 */
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C | C_S,
	C_C | C_S,
	C_C | C_S,
	C_C | C_S,
	C_C | C_S,
	C_C,
	C_C,

	/* 0x10 */
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,
	C_C,

	/* 0x20 */
	C_W | C_S,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,

	/* 0x30 */
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_D,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,

	/* 0x40 */
	C_P,
	C_U | C_X,
	C_U | C_X,
	C_U | C_X,
	C_U | C_X,
	C_U | C_X,
	C_U | C_X,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,

	/* 0x50 */
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_U,
	C_P,
	C_P,
	C_P,
	C_P,
	C_P,

	/* 0x60 */
	C_P,
	C_L | C_X,
	C_L | C_X,
	C_L | C_X,
	C_L | C_X,
	C_L | C_X,
	C_L | C_X,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,

	/* 0x70 */
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_L,
	C_P,
	C_P,
	C_P,
	C_P,
	C_C,

	/* 0x80 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0x90 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xa0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xb0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xc0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xd0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xe0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,

	/* 0xf0 */
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N,
	C_N, C_N, C_N, C_N, C_N, C_N, C_N, C_N 
};


int isalnum(int c)
{
	return tbl[c + 1] & (C_U | C_L | C_D);
}

int isalpha(int c)
{
	return tbl[c + 1] & (C_U | C_L);
}

int iscntrl(int c)
{
	return tbl[c + 1] & (C_C);
}

int isdigit(int c)
{
	return tbl[c + 1] & (C_D);
}

int isgraph(int c)
{
	return tbl[c + 1] & (C_U | C_L | C_D | C_P);
}

int islower(int c)
{
	return tbl[c + 1] & (C_L);
}

int isprint(int c)
{
	return tbl[c + 1] & (C_U | C_L | C_D | C_P | C_W);
}

int ispunct(int c)
{
	return tbl[c + 1] & (C_P);
}

int isspace(int c)
{
	return tbl[c + 1] & (C_S | C_W);
}

int isupper(int c)
{
	return tbl[c + 1] & (C_U);
}

int isxdigit(int c)
{
	return tbl[c + 1] & (C_D | C_X);
}

int tolower(int c)
{
	return isupper(c)? (c + ('a' - 'A')):c;
}

int toupper(int c)
{
	return islower(c)? (c - ('a' - 'A')):c;
}
