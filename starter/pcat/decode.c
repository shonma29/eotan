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
#include "../../lib/librc/rangecoder.h"
#include "../../lib/librc/bit.h"

#define EOF (-1)

static unsigned char *in;
static unsigned char *out;
static size_t in_size;
static size_t out_size;
static unsigned int rpos;
static unsigned int wpos;

static int bgetc(RangeCoder *);
static int bputc(unsigned char ch, RangeCoder *);


static int bgetc(RangeCoder *rc)
{
	return ((rpos < in_size) ? (in[rpos++]) : EOF);
}

static int bputc(unsigned char ch, RangeCoder *rc)
{
	if (wpos < out_size) {
		out[wpos++] = ch;
		return ch;
	} else
		return EOF;
}

int decode(void *dest, void *src, const size_t len, const size_t max)
{
	out = dest;
	in = src;
	in_size = len;
	out_size = max;
	rpos = 0;
	wpos = 0;

	Frequency freq;
	RangeCoder rc;
	rc_initialize(&rc, bgetc, bputc);
	frequency_initialize(&freq);
	return rc_decode(&freq, &rc);
}
