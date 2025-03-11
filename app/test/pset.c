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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <hmi/draw.h>

#define ERR (-1)

#define SPLIT (360)

typedef struct {
	unsigned int x;
	unsigned int y;
	int color;
} point_t;

#define WIDTH (640)
#define HEIGHT (480)

static int _pset(const int, const unsigned int, const unsigned int, const int);
static int _circle(const int, const int, const int, const int);


static int _pset(const int fd, const unsigned int x, const unsigned int y,
		const int color)
{
	char buf[DRAW_PSET_PACKET_SIZE];
	draw_operation_e *ope = (draw_operation_e *) buf;
	*ope = draw_op_pset;

	point_t *point = (point_t *) &(buf[DRAW_OP_SIZE]);
	point->x = x;
	point->y = y;
	point->color = color;

	errno = 0;
	int result = write(fd, buf, sizeof(buf));
	if (result != sizeof(buf)) {
		fprintf(stderr, "write error %d %d\n", result, errno);
		return ERR;
	}

	return 0;
}

static int _circle(const int fd, const int x, const int y, const int radius)
{
	int deg = 0;
	for (int i = 0; i < SPLIT; i++) {
		double rad = deg * M_PI / 180;
		if (_pset(fd, radius * cos(rad) + x, radius * sin(rad) + y,
				0xff00ff))
			return ERR;

		deg += 360 / SPLIT;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int fd = open("/dev/draw", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open error %d\n", errno);
		return EXIT_FAILURE;
	}

	int result = _circle(fd, WIDTH / 2, HEIGHT / 2, HEIGHT / 3);
	close(fd);
	return (result? EXIT_FAILURE : EXIT_SUCCESS);
}
