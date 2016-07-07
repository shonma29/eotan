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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "rangecoder.h"
#include "bit.h"

static int bgetc(RangeCoder *rc);
static int bputc(unsigned char ch, RangeCoder *rc);
static int bputc_1st(unsigned char ch, RangeCoder *rc);


static int bgetc(RangeCoder *rc)
{
	return fgetc(stdin);
}

static int bputc(unsigned char ch, RangeCoder *rc)
{
	return fputc(ch, stdout);
}

static int bputc_1st(unsigned char ch, RangeCoder *rc)
{
	rc->put = bputc;

	return 0;
}

int main(int argc, char **argv)
{
	int result = ERR_OK;
	Frequency *freq = NULL;
	RangeCoder rc;

	do {
		if (argc != 1) {
			fputs("usage: encode < infile > outfile\n", stderr);
			result = ERR_ARG;
			break;
		}

		if (!(freq = (Frequency*)malloc(sizeof(Frequency)))) {
			fputs("cannot allocate memory\n", stderr);
			result = ERR_MEMORY;
			break;
		}

		rc_initialize(&rc, bgetc, bputc_1st);
		frequency_initialize(freq);
		result = rc_encode(freq, &rc);
	} while (false);

	if (freq)
		free(freq);

	return result;
}
