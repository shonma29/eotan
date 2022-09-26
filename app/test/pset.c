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
#include <hmi/draw.h>

#define SPLIT (360)

typedef struct {
	unsigned int x;
	unsigned int y;
	int color;
} point_t;

#define WIDTH (640)
#define HEIGHT (480)

static int _pset(const unsigned int, const unsigned int, const int);
static void _circle(const int, const int, const int);


static int _pset(const unsigned int x, const unsigned int y, const int color)
{
	char buf[DRAW_PSET_PACKET_SIZE];
	draw_operation_e *ope = (draw_operation_e *) buf;
	*ope = draw_pset;

	point_t *point = (point_t *) &(buf[DRAW_OPE_SIZE]);
	point->x = x;
	point->y = y;
	point->color = color;

	fsmsg_t message;
	message.header.ident = IDENT;
	message.header.type = Twrite;
	message.Twrite.fid = DRAW_FID;
	message.Twrite.offset = 0;
	message.Twrite.count = sizeof(buf);
	message.Twrite.data = buf;

	int err = ipc_call(PORT_CONSOLE, &message, MESSAGE_SIZE(Twrite));
	if (err < 0)
		printf("call error %d\n", err);
	else if (message.Rwrite.count < 0)
		printf("draw error %d\n", message.Rwrite.count);

	return err;
}

static void _circle(const int x, const int y, const int radius)
{
	int deg = 0;
	for (int i = 0; i < SPLIT; i++) {
		double rad = deg * M_PI / 180;
		_pset(radius * cos(rad) + x, radius * sin(rad) + y, 0xff00ff);
		deg += 360 / SPLIT;
	}
}

int main(int argc, char **argv)
{

	_circle(WIDTH / 2, HEIGHT / 2, HEIGHT / 3);
	return 0;
}
