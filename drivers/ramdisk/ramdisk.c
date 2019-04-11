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
#include <device.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include "ramdisk.h"

unsigned char buf[BUF_SIZE];

static int check_param(const off_t start, const size_t size);


int detach(void)
{
	return 0;
}

int open(void)
{
	return 0;
}

int close(const int channel)
{
	return ((channel == 0)? 0:(-1));
}

int read(unsigned char *outbuf, const int channel,
		const off_t start, const size_t size)
{
	int result = check_param(start, size);
	if (result)
		return result;

	memcpy(outbuf, &(buf[start]), size);

	return size;
}

int write(unsigned char *inbuf, const int channel,
		const off_t start, const size_t size) {
	int result = check_param(start, size);
	if (result)
		return result;

	memcpy(&(buf[start]), inbuf, size);

	return size;
}

static int check_param(const off_t start, const size_t size)
{
	if (start < 0)
		return -1;

	if (start >= sizeof(buf))
		return -1;

	size_t rest = sizeof(buf) - start;
	if (size > rest)
		return -1;

	if (size > DEV_BUF_SIZE)
		return -1;

	return 0;
}
