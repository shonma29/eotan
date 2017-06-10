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
#include <core.h>
#include <device.h>
#include <services.h>
#include <stdio.h>

typedef struct {
	unsigned int x;
	unsigned int y;
	int color;
} point_t;

static unsigned int width = 640;
static unsigned int height = 480;

int pset(const unsigned int x, const unsigned int y, const int color)
{
	devmsg_t msg;
	ER_UINT err;
	point_t buf;

	if (x >= width)
		return E_PAR;
	if (y >= height)
		return E_PAR;

	buf.x = x;
	buf.y = y;
	buf.color = color;

	msg.Rwrite.operation = operation_write;
	msg.Rwrite.channel = 2;
	msg.Rwrite.offset = 0;
	msg.Rwrite.length = sizeof(buf);
	msg.Rwrite.data = (unsigned char*)&buf;

	err = cal_por(PORT_CONSOLE, 0xffffffff, &msg, sizeof(msg.Rwrite));
	if (err < 0)
		printf("cal_por error %d\n", (int)err);
	return err;
}

int main(int argc, char **argv)
{

	pset(320, 240, 0xff00ff);

	return 0;
}