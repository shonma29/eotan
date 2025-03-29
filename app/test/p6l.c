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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hmi/draw.h>

#define ERR_MEMORY (4)

static struct {
	int op;
	blit_param_t param;
} packet;

static inline int roundup(int x, int y)
{
	return ((x + (y - 1)) & ~(y - 1));
}

static void to_bgr(char *, const size_t);
static int blit(const int);
static int process(const int, const int);


static void to_bgr(char *p, const size_t pixels)
{
	for (size_t i = pixels; i > 0; i--) {
		uint8_t c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += sizeof(Color_Rgb);
	}
}

static int blit(const int out)
{
	packet.op = draw_op_blit;
	errno = 0;

	int result = write(out, &packet, sizeof(packet));
	if (result != sizeof(packet)) {
		fprintf(stderr, "write err %d %d\n", result, errno);
		return (-1);
	}

	return 0;
}

static int process(const int out, const int in)
{
	char buf[3];

	if (read(in, buf, 3) != 3) {
		printf("read err[desc]\n");
		return (-1);
	}

	if (buf[0] != 'P' || buf[1] != '6' || buf[2] != '\n') {
		printf("bad desc\n");
		return (-2);
	}

	size_t width = 0;
	for (;;) {
		if (read(in, buf, 1) != 1) {
			printf("read err[width]\n");
			return (-1);
		}

		if (buf[0] == ' ')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[width]\n");
			return (-2);
		}

		width = width * 10 + buf[0] - '0';
	}

	size_t height = 0;
	for (;;) {
		if (read(in, buf, 1) != 1) {
			printf("read err[height]\n");
			return (-1);
		}

		if (buf[0] == '\n')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[height]\n");
			return (-2);
		}

		height = height * 10 + buf[0] - '0';
	}

	//TODO check overflow of width, height, color_max
	if (width == 0 || width >= 640 || height == 0 || height >= 480) {
		printf("bad size %d, %d\n", width, height);
		return (-2);
	}

	printf("size %d, %d\n", width, height);

	unsigned int color_max = 0;
	for (;;) {
		if (read(in, buf, 1) != 1) {
			printf("read err[color]\n");
			return (-1);
		}

		if (buf[0] == '\n')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[color]\n");
			return (-2);
		}

		color_max = color_max * 10 + buf[0] - '0';
	}

	if (color_max != 255) {
		printf("bad color %d\n", color_max);
		return (-2);
	}

	blit_param_t *par = &(packet.param);
	par->dest.min.x = 0;
	par->dest.min.y = 0;
	//TODO define rect_get_width(Rectangle)
	par->dest.max.x = width;
	//TODO define rect_get_height(Rectangle)
	par->dest.max.y = height;
	//TODO get type and bpp from global Display
	par->type = B8G8R8;

	size_t p6_bpl = width * sizeof(Color_Rgb);
	//TODO define rect_rount_up(int or long)
	par->bpl = roundup(p6_bpl, sizeof(int));
	par->base = malloc(par->bpl * height);
	if (!(par->base)) {
		printf("memory exhausted\n");
		return ERR_MEMORY;
	}

	char *line = par->base;
	for (int i = 0; i < height; i++) {
		if (read(in, line, p6_bpl) != p6_bpl) {
			printf("read err[rest]\n");
			free(par->base);
			return (-1);
		}

		to_bgr(line, width);
		line += par->bpl;
	}

	if (blit(out) < 0) {
		free(par->base);
		return (-3);
	}

	free(par->base);
	printf("done\n");
	return 0;
}

int main(int argc, char **argv)
{
	int result = 0;
	if (argc == 2) {
		errno = 0;

		int in = open(argv[1], O_RDONLY);
		if (in >= 0) {
			int out = open("/dev/draw", O_WRONLY);
			if (out < 0)
				fprintf(stderr, "open 'draw' err %d\n", errno);
			else {
				result = process(out, in);
				close(in);
			}

			close(out);
		} else
			printf("open %s err %d\n", argv[1], errno);
	} else
		printf("arg err\n");

	return result;
}
