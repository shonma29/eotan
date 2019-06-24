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
#include <boot/initrd.h>
#include <mpu/memory.h>
#include "../../lib/librc/rangecoder.h"
#include "../../lib/librc/bit.h"

#define EOF (-1)

static size_t image_size;
static unsigned char *image_buf;
static unsigned int rpos;
static unsigned int wpos;

static int bgetc(RangeCoder *rc);
static int bputc(unsigned char ch, RangeCoder *rc);


static int bgetc(RangeCoder *rc)
{
	return ((rpos < image_size) ? (image_buf[rpos++]) : EOF);
}

static int bputc(unsigned char ch, RangeCoder *rc)
{
	if (wpos < INITRD_SIZE) {
		//TODO get from parameter
		unsigned char *buf = kern_v2p((unsigned char*)(INITRD_ADDR));
		buf[wpos++] = ch;
		return ch;
	} else
		return EOF;
}

int decode(unsigned char *address, const size_t size)
{
	Frequency freq;
	RangeCoder rc;

	image_buf = address;
	image_size = size;

	rc_initialize(&rc, bgetc, bputc);
	frequency_initialize(&freq);

	return rc_decode(&freq, &rc);
}
