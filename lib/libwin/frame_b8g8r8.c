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
#include <fcntl.h>
#include <unistd.h>
#include <libc.h>
#include <console.h>
#include <win/window.h>

#define PADDING_LEFT (2)
#define PADDING_RIGHT (3)
#define PADDING_TOP (2)
#define PADDING_BOTTOM (3)
#define PADDING_TITLE (13)

#define TITLE_X (2)
#define TITLE_Y (2)

#define COLOR_LIGHT_GRAY 0xdfe3e4
#define COLOR_DARK_GRAY 0x24130d

enum {
	ERR_MEMORY = -1,
	ERR_BIND = -2,
	ERR_OPEN_EVENT = -3,
	ERR_OPEN_DRAW = -4,
	ERR_WRITE_DRAW = -5
};

static Color_Rgb title_color[] = {
	{ 0xe4, 0xe3, 0xdf },
	{ 0x30, 0x30, 0x30 }
};

static void _set_inner(Rectangle * const, Rectangle const * const,
		uint32_t const);

int window_initialize(Window ** const wp, int const width, int const height,
	uint32_t const attr)
{
	//TODO 'bind' returns positive integer
	if (bind("#i", "/dev", MREPL) < 0)
		return ERR_BIND;

	Window *w = malloc(sizeof(Window) + sizeof(Color_Rgb) * width * height);
	if (!w)
		return ERR_MEMORY;

	w->event_fd = open("/dev/event", O_RDONLY);
	if (w->event_fd < 0) {
		free(w);
		return ERR_OPEN_EVENT;
	}

	w->draw_fd = open("/dev/draw", O_WRONLY);
	if (w->draw_fd < 0) {
		free(w);
		return ERR_OPEN_DRAW;
	}

	Display *d = &(w->display);
	d->r.min.x = 0;
	d->r.min.y = 0;
	d->r.max.x = width;
	d->r.max.y = height;
	d->base = &(w[1]);
	d->bpl = width * sizeof(Color_Rgb);
	d->bpp = sizeof(Color_Rgb);
	d->type = B8G8R8;

	w->attr = attr;
	_set_inner(&(w->inner), &(d->r), w->attr);

	w->packet.op = draw_op_blit;
	w->packet.param.bpl = w->display.bpl;
	w->packet.param.type = w->display.type;

	*wp = w;
	return 0;
}

static void _set_inner(Rectangle * const inner, Rectangle const * const outer,
		uint32_t const attr)
{
	*inner = *outer;

	if (attr & WINDOW_ATTR_WINDOW) {
		inner->min.x += PADDING_LEFT;
		inner->min.y += PADDING_TOP + PADDING_TITLE;
		inner->max.x -= PADDING_RIGHT;
		inner->max.y -= PADDING_BOTTOM;
	}
}

void window_set_title(Window * const w, char const * const title)
{
	w->title = title;
}

void window_draw_frame(Window const * const w)
{
	if (w->attr & WINDOW_ATTR_WINDOW) {
		Rectangle const *outer = &(w->display.r);
		Rectangle const *inner = &(w->inner);
		int outer_width = outer->max.x - outer->min.x;
		int outer_height = outer->max.y - outer->min.y;
		int padding_top = inner->min.y - outer->min.y;
		Rectangle r;

		// border top
		r.min.x = 0;
		r.min.y = 0;
		r.max.x = outer_width - 1;
		r.max.y = padding_top - 1;
		draw_fill(&(w->display), &r, COLOR_LIGHT_GRAY);

		r.min.y = padding_top - 1;
		r.max.y = padding_top;
		draw_fill(&(w->display), &r, COLOR_DARK_GRAY);

		// border left
		r.min.y = padding_top;
		r.max.x = PADDING_LEFT;
		r.max.y = inner->max.y - outer->min.y;
		draw_fill(&(w->display), &r, COLOR_LIGHT_GRAY);

		// border bottom
		r.min.y = inner->max.y - outer->min.y;
		r.max.x = outer_width - 1;
		r.max.y = outer_height - 1;
		draw_fill(&(w->display), &r, COLOR_LIGHT_GRAY);

		r.min.y = outer_height - 1;
		r.max.y = outer_height;
		draw_fill(&(w->display), &r, COLOR_DARK_GRAY);

		// border right
		r.min.x = inner->max.x - outer->min.x;
		r.min.y = padding_top;
		r.max.y = inner->max.y - outer->min.y;
		draw_fill(&(w->display), &r, COLOR_LIGHT_GRAY);

		r.min.x = outer_width - 1;
		r.min.y = 0;
		r.max.x = outer_width;
		r.max.y = outer_height;
		draw_fill(&(w->display), &r, COLOR_DARK_GRAY);

		if (w->title)
			draw_string(&(w->display), TITLE_X, TITLE_Y,
					title_color, &default_font, w->title);
	}
}

int window_blit(Window * const w, Rectangle const * const r)
{
	blit_param_t *par = &(w->packet.param);
	par->dest = *r;
	par->base = (void *) ((uintptr_t) (w->display.base)
			+ r->min.y * w->display.bpl
			+ r->min.x * w->display.bpp);

	int result = write(w->draw_fd, &(w->packet), sizeof(w->packet));
	if (result != sizeof(w->packet))
		return ERR_WRITE_DRAW;

	return 0;
}
#if 0
void _set_blue(Screen *s)
{
	s->fgcolor.rgb.b = 0xff;
	s->fgcolor.rgb.g = 0xff;
	s->fgcolor.rgb.r = 0xff;

	s->bgcolor.rgb.b = 0x7f;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 0;
}

void _set_green(Screen *s)
{
	s->fgcolor.rgb.b = 31;
	s->fgcolor.rgb.g = 223;
	s->fgcolor.rgb.r = 0;

	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 31;
	s->bgcolor.rgb.r = 0;
}

void _set_orange(Screen *s)
{
	s->fgcolor.rgb.b = 0;
	s->fgcolor.rgb.g = 127;
	s->fgcolor.rgb.r = 255;

	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 31;
}

void _set_white(Screen *s)
{
	s->fgcolor.rgb.b = 0x30;
	s->fgcolor.rgb.g = 0x30;
	s->fgcolor.rgb.r = 0x30;

	s->bgcolor.rgb.b = 0xfc;
	s->bgcolor.rgb.g = 0xfc;
	s->bgcolor.rgb.r = 0xfc;
}

void _set_gray(Screen *s)
{
	s->fgcolor.rgb.b = 40;
	s->fgcolor.rgb.g = 66;
	s->fgcolor.rgb.r = 30;

	s->bgcolor.rgb.b = 228;
	s->bgcolor.rgb.g = 227;
	s->bgcolor.rgb.r = 223;
}
#endif
