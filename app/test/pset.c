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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <hmi/draw.h>

#define ERR (-1)

#define SPLIT (360)

static struct {
	int op;
	blit_param_t param;
} packet;

#define WIDTH (640)
#define HEIGHT (480)

extern Font default_font;

static int _blit(const int, const void *);
static void _circle(char *, const int, const int, const int);
static void _string(char *);


static int _blit(const int fd, const void *buf)
{
	packet.op = draw_op_blit;

	blit_param_t *par = &(packet.param);
	par->base = (void *) buf;
	par->dest.min.x = 0;
	par->dest.min.y = 0;
	par->dest.max.x = WIDTH;
	par->dest.max.y = HEIGHT;
	par->bpl = WIDTH * sizeof(Color_Rgb);
	//TODO get type and bpp from global Display
	par->type = B8G8R8;

	errno = 0;
	int result = write(fd, &packet, sizeof(packet));
	if (result != sizeof(packet)) {
		fprintf(stderr, "write error %d %d\n", result, errno);
		return ERR;
	}

	return 0;
}

static void _circle(char *buf, const int x, const int y, const int radius)
{
	int deg = 0;
	for (int i = 0; i < SPLIT; i++) {
		double rad = deg * M_PI / 180;
		int x1 = (radius * cos(rad) + x);
		int y1 = (radius * sin(rad) + y);
		uintptr_t addr = (uintptr_t) buf
				+ y1 * WIDTH * sizeof(Color_Rgb)
				+ x1 * sizeof(Color_Rgb);
		char *p = (char *) addr;
		p[0] = 0xff;
		p[1] = 0x00;
		p[2] = 0xff;

		deg += 360 / SPLIT;
	}
}

static void _string(char *buf)
{
	Display display = {
		{ { 0, 0 }, { WIDTH, HEIGHT } },
		buf,
		WIDTH * sizeof(Color_Rgb),
		sizeof(Color_Rgb),
		B8G8R8
	};
	Color_Rgb c[] = {
		{ 0xff, 0xff, 0xff },
		{ 0xff, 0x00, 0x00 }
	};
	draw_string(&display, 25, 48, c, &default_font, "hello, world");
}

int main(int argc, char **argv)
{
	int fd = open("/dev/draw", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open error %d\n", errno);
		return EXIT_FAILURE;
	}

	char *buf = malloc(WIDTH * HEIGHT * sizeof(Color_Rgb));
	if (!buf) {
		fprintf(stderr, "memory exhausted\n");
		return ERR;
	}

	memset(buf, 0xff, WIDTH * HEIGHT * sizeof(Color_Rgb));
	_circle(buf, WIDTH / 2, HEIGHT / 2, HEIGHT / 3);
	_string(buf);

	int result = _blit(fd, buf);
	free(buf);
	close(fd);
	return (result? EXIT_FAILURE : EXIT_SUCCESS);
}
