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
#include <stdio.h>
#include <unistd.h>

#define NOT_CACHE (-2)

#define ERR_UNKNOWN (1)

static int cache = NOT_CACHE;

static int bgetc(FILE *in);
static int bungetc(int c);
static int encode(FILE *out, FILE *in);


static int bgetc(FILE *in)
{
	if (cache == NOT_CACHE)
		return fgetc(in);

	int c = cache;
	cache = NOT_CACHE;
	return c;
}

static int bungetc(const int c)
{
	if (cache == NOT_CACHE) {
		cache = c;
		return 0;
	}

	return -1;
}

static int encode(FILE *out, FILE *in)
{
	for (int c; (c = bgetc(in)) != EOF;) {
		switch (c) {
		case 0:
		{
			int next;
			size_t count;
			for (count = 1; !(next = bgetc(in)); count++);

			if (bungetc(next))
				return ERR_UNKNOWN;

			for (count++; count != 1; count >>= 1)
				fputc(count & 1, out);
		}
			break;
		case 0xfe:
			fputc(0xff, out);
			fputc(0, out);
			break;
		case 0xff:
			fputc(0xff, out);
			fputc(1, out);
			break;
		default:
			fputc(c + 1, out);
			break;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	return encode(stdout, stdin);
}
