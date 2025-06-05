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
#include <nerve/kcall.h>
#include "hmi.h"
#include "mouse.h"
#include "session.h"

#define COLOR_ROOT 0x0

static slab_t window_slab;
static Point last_point = { -1, -1 };
static event_message_t mouse_message = { event_mouse, 0 };


int window_initialize(void)
{
	window_slab.unit_size = sizeof(window_t);
	window_slab.block_size = PAGE_SIZE;
	window_slab.min_block = 1;
	window_slab.max_block = slab_max_block(WINDOW_MAX, PAGE_SIZE,
			sizeof(window_t));
	window_slab.palloc = kcall->palloc;
	window_slab.pfree = kcall->pfree;
	slab_create(&window_slab);
	return 0;
}
//TODO !split window design
int window_create(window_t **w, const int x1, const int y1,
		const int x2, const int y2, const int attr)
{
	int error_no = 0;
	do {
		window_t *p = (window_t *) slab_alloc(&window_slab);
		if (!p) {
			error_no = ENOMEM;
			break;
		}

		p->attr = attr;
		//TODO check minimum rectangle
		p->outer.r.min.x = x1;
		p->outer.r.min.y = y1;
		p->outer.r.max.x = x2;
		p->outer.r.max.y = y2;
		rect_normalize(&(p->outer.r));
		p->outer.viewport = p->outer.r;

		int padding_left = 0;
		int padding_right = 0;
		int padding_top = 0;
		int padding_bottom = 0;
		if (attr & WINDOW_ATTR_HAS_BORDER) {
			padding_left += 2;
			padding_right += 3;
			padding_top += 2;
			padding_bottom += 3;
		}

		if (attr & WINDOW_ATTR_HAS_TITLE)
			padding_top += 13;

		if (attr & WINDOW_ATTR_SCROLLABLE_Y)
			padding_right += 12;

		if (attr & WINDOW_ATTR_SCROLLABLE_X)
			padding_bottom += 12;

		p->inner.r = p->outer.r;
		p->inner.r.min.x += padding_left;
		p->inner.r.min.y += padding_top;
		p->inner.r.max.x -= padding_right;
		p->inner.r.max.y -= padding_bottom;
		p->inner.viewport = p->inner.r;
		*w = p;
	} while (false);

	return error_no;
}

int window_destroy(window_t *w)
{
	int error_no = 0;
	do {
		mouse_hide();
		draw_fill(display, &(w->outer.viewport), COLOR_ROOT);
		mouse_show();
		slab_free(&window_slab, w);
	} while (false);

	return error_no;
}

void window_focus(const int data)
{
	if (data >= 0) {
#if 0
		int buttons = (data >> 24) & 7;
		if (buttons & 1)
			kcall->printk("<left>");
		else if (buttons & 2)
			kcall->printk("<right>");
		else if (buttons & 4)
			kcall->printk("<middle>");
#endif
		if (data & 0x40000000) {
			mouse_hide();
			mouse_show();
		}

		last_point.x = (data >> 12) & 0xfff;
		last_point.y = data & 0xfff;
	}

	for (list_t *p = list_next(&session_list);
			!list_is_edge(&session_list, p); p = p->next) {
		session_t *s = getSessionFromList(p);
		window_t *w = s->window;
		if ((w->outer.r.min.x <= last_point.x)
			&& (w->outer.r.min.y <= last_point.y)
			&& (last_point.x < w->outer.r.max.x)
			&& (last_point.y < w->outer.r.max.y)
		) {
#if 0
			if (focused_session == s)
				//TODO send 'move' to current focused session
#endif
			if (focused_session != s)
				//TODO send 'out' to previously focused session
				//TODO send 'in' to current focused session
				focused_session = s;

			mouse_message.data = mouse_encode_data(
					data >> 24,
					last_point.x - w->outer.r.min.x,
					last_point.y - w->outer.r.min.y);
			event_enqueue(&mouse_message);
			return;
		}
	}

	if (focused_session)
		//TODO send 'out' to old focus
		focused_session = NULL;
}
