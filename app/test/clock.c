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
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <win/window.h>

#define MYNAME "clock"

#define WIDTH (512)
#define HEIGHT (374)

extern Font default_font;

static Color_Rgb text_color[] = {
	{ 0xff, 0xff, 0xff },
	{ 0xff, 0x00, 0x00 }
};

extern void __malloc_initialize(void);


void _main(int argc, char **argv, char **env)
{
	errno = 0;
	__malloc_initialize();

	Window *w;
	if (window_initialize(&w, WIDTH, HEIGHT, WINDOW_ATTR_WINDOW))
		_exit(EXIT_FAILURE);

	draw_fill(&(w->display), &(w->inner), 0xffffff);
	window_set_title(w, MYNAME);
	window_draw_frame(w);

	do {
		time_t t = time(NULL);
		if (t == -1)
			break;//TODO show error

		struct tm tm;
		if (!gmtime_r(&t, &tm))
			break;//TODO show error

		char text[256];//TODO ugly
		size_t len = strftime(text, sizeof(text),
				"%a %b %d %H:%M:%S", &tm);
		if (!len)
			break;//TODO show error

		len++;

		draw_string(&(w->display), 25, 48, text_color, &default_font,
				text);

		if (window_blit(w, &(w->display.r)))
			break;

		//if (tm.tm_sec < 60)
		//	sleep(60 - tm.tm_sec);
		sleep(1);
	} while (true);

	_exit(EXIT_FAILURE);
}
