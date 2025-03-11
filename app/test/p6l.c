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
#include <unistd.h>
#include <hmi/draw.h>

#define MAX_PIXEL (337)
#define MAX_BYTES (MAX_PIXEL * sizeof(Color_Rgb))
#define BUF_SIZE (DRAW_PUT_PACKET_SIZE + MAX_BYTES)

static uint8_t buf[BUF_SIZE];

static void to_bgr(const size_t);
static int putline(const int, const size_t, uint8_t *);
static int process(const int, const int);


static void to_bgr(const size_t pixels)
{
	uint8_t *p = &buf[DRAW_PUT_PACKET_SIZE];
	for (size_t i = pixels; i > 0; i--) {
		uint8_t c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += sizeof(Color_Rgb);
	}
}

static int putline(const int out, const size_t bytes, uint8_t *buf)
{
	errno = 0;

	int result = write(out, buf, bytes);
	if (result != bytes) {
		fprintf(stderr, "write err %d %d\n", result, errno);
		return (-1);
	}

	return 0;
}

static int process(const int out, const int in)
{
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

	unsigned char *data = &(buf[DRAW_PUT_PACKET_SIZE]);
	int *packet = (int *) &(buf[DRAW_OP_SIZE]);
	draw_operation_e *ope = (draw_operation_e *) buf;
	*ope = draw_op_put;

	for (int i = 0; i < height; i++) {
		size_t rest = width;
		int j = 0;
		while (rest > MAX_PIXEL) {
			if (read(in, data, MAX_BYTES) != MAX_BYTES) {
				printf("read err[block]\n");
				return (-1);
			}

			to_bgr(MAX_PIXEL);

			packet[0] = j;
			packet[1] = i;
			if (putline(out, sizeof(buf), buf) < 0)
				return (-3);

			j += MAX_PIXEL;
			rest -= MAX_PIXEL;
		}

		if (rest) {
			size_t bytes = rest * sizeof(Color_Rgb);
			if (read(in, data, bytes) != bytes) {
				printf("read err[rest]\n");
				return (-1);
			}

			to_bgr(rest);

			packet[0] = j;
			packet[1] = i;
			if (putline(out, DRAW_PUT_PACKET_SIZE + bytes, buf) < 0)
				return (-3);
		}
	}

	printf("done\n");
	return 0;
}

int main(int argc, char **argv)
{
	if (argc == 2) {
		errno = 0;

		int in = open(argv[1], O_RDONLY);
		if (in >= 0) {
			int out = open("/dev/draw", O_WRONLY);
			if (out < 0)
				fprintf(stderr, "open 'draw' err %d\n", errno);
			else {
				process(out, in);
				close(in);
			}

			close(out);
		} else
			printf("open %s err %d\n", argv[1], errno);
	} else
		printf("arg err\n");

	return 0;
}
