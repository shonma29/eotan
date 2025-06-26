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
#include <string.h>
#include <unistd.h>
#include <event.h>
#include <win/window.h>

#define MYNAME "p6l"

#define BUF_SIZE (8192)

#define WIDTH (512)
#define HEIGHT (374)

static void _show_error(char const *);
static void _to_bgr(char *, const size_t);
static int _load(Window const * const, const int);
static int _window_main(Window const * const);


void _put_error(const char *message) {
	write(STDERR_FILENO, message, strlen(message));
}

static void _show_error(char const *message) {
	char *error_string = strerror(errno);
	_put_error(MYNAME ": ");
	_put_error(message);
	_put_error(" ");
	_put_error(error_string);
	_put_error("\n");
}

static void _to_bgr(char *p, const size_t pixels)
{
	for (size_t i = pixels; i > 0; i--) {
		uint8_t c = p[0];
		p[0] = p[2];
		p[2] = c;
		p += sizeof(Color_Rgb);
	}
}

static int _load(Window const * const w, const int in)
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
	//TODO clip
	if (width == 0 || width > (w->inner.max.x - w->inner.min.x)
			|| height == 0
			|| height > (w->inner.max.y - w->inner.min.y)) {
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

	size_t p6_bpl = width * sizeof(Color_Rgb);
	char *line = w->display.base
			+ w->inner.min.y * w->display.bpl
			+ w->inner.min.x * w->display.bpp;
	for (int i = 0; i < height; i++) {
		if (read(in, line, p6_bpl) != p6_bpl) {
			printf("read err[rest]\n");
			return (-1);
		}

		_to_bgr(line, width);
		line += w->display.bpl;
	}

	return 0;
}

static int _window_main(Window const * const w)
{
	for (;;) {
		errno = 0;

		event_message_t message[BUF_SIZE / sizeof(event_message_t)];
		ssize_t len = read(w->event_fd, message, sizeof(message));
		if ((len <= 0)
				|| (len % sizeof(event_message_t))) {
			_show_error("failed to read event.");
			return EXIT_FAILURE;
		}

		len /= sizeof(event_message_t);
		for (int i = 0; i < len; i++) {

			if (message[i].type != event_mouse)
				continue;

			int buttons = (message[i].data >> 24) & 7;
			if (buttons & 1)
				return EXIT_SUCCESS;
		}
		sleep(1);
	}
}

int main(int argc, char **argv)
{
	int result = EXIT_FAILURE;
	errno = 0;
	do {
		Window *w;

		//TODO get global Display and window
		if (window_initialize(&w, WIDTH, HEIGHT, WINDOW_ATTR_WINDOW)) {
			_put_error(MYNAME ": failed to open window\n");
			break;
		}

		//TODO use 'draw_fill'
		memset(w->display.base, 0, WIDTH * HEIGHT * sizeof(Color_Rgb));
		window_set_title(w, MYNAME);
		window_draw_frame(w);

		if (argc != 2) {
			_put_error("usage: " MYNAME " filename\n");
			break;
		}

		errno = 0;

		int in = open(argv[1], O_RDONLY);
		if (in < 0) {
			_show_error("failed to open file.");
			break;
		}

		result = _load(w, in);
		close(in);

		if (result)
			break;

		result = window_blit(w, &(w->display.r));
		if (result)
			break;

		result = _window_main(w);
	} while (false);

	exit(result);
}
