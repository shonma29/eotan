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
#include <features.h>
#ifdef USE_VESA
#include <errno.h>
#include <nerve/kcall.h>
#include "hmi.h"

#define WINDOW_MAX (32)
#define SCREEN7_HEIGHT (20)

#define getParent(type, p) ((uintptr_t) p - offsetof(type, node))

static slab_t window_slab;
static tree_t window_tree;
static list_t window_list;

Display *display;
static Screen screen2;
static Screen screen7;
esc_state_t state2;
esc_state_t state7;


int window_initialize(void)
{
	display = get_display();
	if (!display)
		return E_SYS;

	window_slab.unit_size = sizeof(window_t);
	window_slab.block_size = PAGE_SIZE;
	window_slab.min_block = 1;
	window_slab.max_block = slab_max_block(WINDOW_MAX, PAGE_SIZE,
			sizeof(window_t));
	window_slab.palloc = kcall->palloc;
	window_slab.pfree = kcall->pfree;
	slab_create(&window_slab);
	tree_create(&window_tree, NULL, NULL);
	list_initialize(&window_list);

	window_t *w;
	int width = display->r.max.x - display->r.min.x;
	int height = display->r.max.y - display->r.min.y;
	create_window(&w, 0, SCREEN7_HEIGHT, width / 2,
			SCREEN7_HEIGHT + (height - SCREEN7_HEIGHT) / 2,
			WINDOW_ATTR_HAS_BORDER | WINDOW_ATTR_HAS_TITLE,
			"Console", &screen0);
	terminal_write(STR_CONS_INIT, &state0, 0, LEN_CONS_INIT);

	state2.screen = &screen2;
	terminal_initialize(&state2);
	screen2.fgcolor.rgb.b = 0;
	screen2.fgcolor.rgb.g = 127;
	screen2.fgcolor.rgb.r = 255;
	screen2.bgcolor.rgb.b = 0;
	screen2.bgcolor.rgb.g = 0;
	screen2.bgcolor.rgb.r = 31;
	create_window(&w, 0,
			SCREEN7_HEIGHT + (height - SCREEN7_HEIGHT) / 2,
			width / 2,
			SCREEN7_HEIGHT + ((height - SCREEN7_HEIGHT) / 2) * 2,
			WINDOW_ATTR_HAS_BORDER | WINDOW_ATTR_HAS_TITLE,
			"Draw", &screen2);
	terminal_write(STR_CONS_INIT, &state2, 0, LEN_CONS_INIT);

	state7.screen = &screen7;
	terminal_initialize(&state7);
	screen7.fgcolor.rgb.b = 40;
	screen7.fgcolor.rgb.g = 66;
	screen7.fgcolor.rgb.r = 30;
	screen7.bgcolor.rgb.b = 228;
	screen7.bgcolor.rgb.g = 227;
	screen7.bgcolor.rgb.r = 223;
	create_window(&w, 0, 0, width, SCREEN7_HEIGHT,
			WINDOW_ATTR_HAS_BORDER,
			NULL, &screen7);
	terminal_write(STR_CONS_INIT, &state7, 0, LEN_CONS_INIT);
	return 0;
}

int create_window(window_t **w, const int x1, const int y1,
		const int x2, const int y2, const int attr, const char *title,
		Screen *s)
{
	int error_no = 0;
	do {
		int wid;
		for (wid = 1; wid <= WINDOW_MAX; wid++)
			if (!tree_get(&window_tree, wid))
				break;

		if (wid >= WINDOW_MAX) {
			error_no = ENOMEM;
			break;
		}

		window_t *p = (window_t *) slab_alloc(&window_slab);
		if (!p) {
			error_no = ENOMEM;
			break;
		}

		if (!tree_put(&window_tree, wid, (node_t *) p)) {
			slab_free(&window_slab, p);
			error_no = EBUSY;
			break;
		}

		p->attr = attr;
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

		s->base = (void *) ((uintptr_t) display->base
				+ p->inner.r.min.y * display->bpl
				+ p->inner.r.min.x * display->bpp);
		s->p = (uint8_t *) (s->base);
		s->width = p->inner.r.max.x - p->inner.r.min.x;
		s->height = p->inner.r.max.y - p->inner.r.min.y;
		s->chr_width = s->width / s->font.width;
		s->chr_height = s->height / s->font.height;

		list_insert(&window_list, &(p->brothers));
		*w = p;

		int outer_width = p->outer.r.max.x - p->outer.r.min.x;
		int outer_height = p->outer.r.max.y - p->outer.r.min.y;

		if (padding_top) {
			if (attr & WINDOW_ATTR_HAS_TITLE) {
				Rectangle r;
				r.min.x = 0;
				r.min.y = 0;
				r.max.x = outer_width;
				r.max.y = padding_top - 1;
				draw_fill(&(p->outer), &r, 0xdfe3e4);

				r.min.x = 0;
				r.min.y = padding_top - 1;
				r.max.x = outer_width;
				r.max.y = padding_top;
				draw_fill(&(p->outer), &r, 0x24130d);
			} else {
				Rectangle r;
				r.min.x = 0;
				r.min.y = 0;
				r.max.x = outer_width;
				r.max.y = padding_top;
				draw_fill(&(p->outer), &r, 0xdfe3e4);
			}
		}

		if (padding_left) {
			Rectangle r;
			r.min.x = 0;
			r.min.y = padding_top;
			r.max.x = padding_left;
			r.max.y = outer_height - padding_bottom;
			draw_fill(&(p->outer), &r, 0xdfe3e4);
		}

		if (padding_bottom) {
			Rectangle r;
			r.min.x = 0;
			r.min.y = outer_height - padding_bottom;
			r.max.x = outer_width - 1;
			r.max.y = outer_height - 1;
			draw_fill(&(p->outer), &r, 0xdfe3e4);

			r.min.x = 0;
			r.min.y = outer_height - 1;
			r.max.x = outer_width - 1;
			r.max.y = outer_height;
			draw_fill(&(p->outer), &r, 0x24130d);
		}

		if (padding_right) {
			Rectangle r;
			r.min.x = outer_width - padding_right;
			r.min.y = padding_top;
			r.max.x = outer_width - 1;
			r.max.y = outer_height - padding_bottom;
			draw_fill(&(p->outer), &r, 0xdfe3e4);

			r.min.x = outer_width - 1;
			r.min.y = 0;
			r.max.x = outer_width;
			r.max.y = outer_height;
			draw_fill(&(p->outer), &r, 0x24130d);
		}

		if ((attr & WINDOW_ATTR_HAS_TITLE)
				&& title) {
			Color_Rgb c[] = {
				{ 0xe4, 0xe3, 0xdf },
				{ 0x00, 0x00, 0x00 }
			};
			draw_string(&(p->outer), 2, 2, c,
					&default_font, (uint8_t *) title);
		}
	} while (false);

	return error_no;
}

window_t *find_window(const int wid)
{
	node_t *node = tree_get(&window_tree, wid);
	return (node ? (window_t *) getParent(window_t, node) : NULL);
}
#if 0
int remove_window(const int wid)
{
	int error_no = 0;
	do {
		window_t *p = find_window(wid);
		if (!p) {
			//TODO really?
			error_no = EPERM;
			break;
		}

		list_remove(&(p->brothers));
		if (!tree_remove(&window_tree, p->node.key)) {
			//TODO what to do?
		}

		slab_free(&window_slab, p);
	} while (false);

	return error_no;
}
#endif
#endif
