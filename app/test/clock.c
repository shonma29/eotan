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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>
#include <hmi/draw.h>

#define ERR (-1)

#define WIDTH (640)
#define HEIGHT (480)

extern Font default_font;

static struct {
	int op;
	blit_param_t param;
} packet;

static int _blit(const int, const Display *);
static void _string(const Display *, const char *);


static int _blit(const int fd, const Display *display)
{
	packet.op = draw_op_blit;

	blit_param_t *par = &(packet.param);
	par->dest = display->r;
	par->base = display->base;
	par->bpl = display->bpl;
	par->type = display->type;

	errno = 0;
	int result = write(fd, &packet, sizeof(packet));
	if (result != sizeof(packet)) {
		fprintf(stderr, "write error %d %d\n", result, errno);
		return ERR;
	}

	return 0;
}

static void _string(const Display *display, const char *str)
{
	Color_Rgb c[] = {
		{ 0xff, 0xff, 0xff },
		{ 0xff, 0x00, 0x00 }
	};
	draw_string(display, 25, 48, c, &default_font, str);
}

int main(int argc, char **argv)
{
	errno = 0;
	if (bind("#i", "/mnt", MREPL) < 0) {
		fprintf(stderr, "cannot bind /mnt %d\n", errno);
		return EXIT_FAILURE;
	}

	int event = open("/mnt/event", O_RDONLY);
	if (event < 0) {
		fprintf(stderr, "cannot open /mnt/event %d\n", errno);
		return EXIT_FAILURE;
	}

	int draw = open("/mnt/draw", O_WRONLY);
	if (draw < 0) {
		fprintf(stderr, "cannot open /mnt/draw %d\n", errno);
		return EXIT_FAILURE;
	}

	char *buf = malloc(WIDTH * HEIGHT * sizeof(Color_Rgb));
	if (!buf) {
		fprintf(stderr, "memory exhausted\n");
		return ERR;
	}

	memset(buf, 0xff, WIDTH * HEIGHT * sizeof(Color_Rgb));

	//TODO get global Display
	Display display = {
		{ { 0, 0 }, { WIDTH, HEIGHT } },
		buf,
		WIDTH * sizeof(Color_Rgb),
		sizeof(Color_Rgb),
		B8G8R8
	};

	do {
		time_t t = time(NULL);
		if (t == -1)
			break;//TODO show error

		struct tm tm;
		if (!gmtime_r(&t, &tm))
			break;//TODO show error

		char text[256];//TODO ugly
		size_t len = strftime(text, sizeof(text),
				"%a %b %d %H:%M", &tm);
		if (!len)
			break;//TODO show error

		len++;
		_string(&display, text);
		_blit(draw, &display);

		if (tm.tm_sec < 60)
			sleep(60 - tm.tm_sec);
	} while (true);

	free(buf);
	close(draw);
	close(event);
	unmount(NULL, "/mnt");
	_exit(EXIT_FAILURE);
}
