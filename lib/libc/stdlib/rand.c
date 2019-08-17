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
#include <stdint.h>
#include <stdlib.h>

#define INT_BIT ((CHAR_BIT) * sizeof(int))

static unsigned int x = 123456789;
static unsigned int y = 362436069;
static unsigned int z = 521288629;
static unsigned int w = 88675123;

static unsigned int rotate(const unsigned int, const unsigned int);


/**
 * get rondom value by Xorshift algorithm (George Marsaglia).
 */
int rand(void)
{
	unsigned int t = x ^ (x << 11);

	x = y;
	y = z;
	z = w;

	return (int)((w = (w ^ (w >> 19)) ^ (t ^ (t >> 8))) & RAND_MAX);
}

static unsigned int rotate(const unsigned int v, const unsigned int n)
{
	unsigned int lmask = (1 << n) - 1;
	unsigned int rmask = (1 << (INT_BIT - n)) - 1;

	return ((v & lmask) << (INT_BIT - n)) | ((v >> n) & rmask);
}

void srand(unsigned int seed)
{
	x ^= seed;
	y ^= rotate(seed, 4);
	z ^= rotate(seed, 8);
	w ^= rotate(seed, 12);
}
