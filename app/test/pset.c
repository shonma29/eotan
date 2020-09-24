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
#include <errno.h>
#include <ipc.h>
#include <math.h>
#include <services.h>
#include <stdio.h>
#include <fs/protocol.h>

#define SPLIT (360)

typedef struct {
	unsigned int x;
	unsigned int y;
	int color;
} point_t;

static size_t width = 640;
static size_t height = 480;

static int pset(const unsigned int, const unsigned int, const int);


static int pset(const unsigned int x, const unsigned int y, const int color)
{
	if (x >= width)
		return EINVAL;

	if (y >= height)
		return EINVAL;

	point_t buf;
	buf.x = x;
	buf.y = y;
	buf.color = color;

	fsmsg_t message;
	message.header.ident = IDENT;
	message.header.type = Twrite;
	message.Twrite.fid = 5;
	message.Twrite.offset = 0;
	message.Twrite.count = sizeof(buf);
	message.Twrite.data = (char *) &buf;

	int err = ipc_call(PORT_CONSOLE, &message, MESSAGE_SIZE(Twrite));
	if (err < 0)
		printf("call error %d\n", err);

	return err;
}

int main(int argc, char **argv)
{
	int deg = 0;
	for (int i = 0; i < SPLIT; i++) {
		double rad = deg * M_PI / 180;
		pset(height / 3 * cos(rad) + width / 2,
				height / 3 * sin(rad) + height / 2,
				0xff00ff);
		deg += 360 / SPLIT;
	}

	return 0;
}
