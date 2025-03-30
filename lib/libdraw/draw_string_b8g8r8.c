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

#ifdef PIXEL_BGR
#include <limits.h>
#include <hmi/draw.h>
#include <nerve/global.h>

#define FONT_BITS CHAR_BIT

static inline size_t font_bytes(const size_t width)
{
	return ((width + (CHAR_BIT - 1)) / CHAR_BIT);
}

static bool _putc(const Display *, uint8_t *, int *, const Color_Rgb *,
		Font *, const uint8_t, int, size_t);
static uint8_t *get_font_address(const Font *, const uint8_t);


void draw_string(const Display *display, const int x, const int y,
		const Color_Rgb *color, Font *font, const char *str)
{
	int y1 = y;
	if (y1 >= display->r.max.y)
		return;

	int y2 = y1 + font->height;
	if (y2 <= display->r.min.y)
		return;

	int start_y;
	if (y1 < display->r.min.y) {
		start_y = display->r.min.y - y1;
		y1 = display->r.min.y;
	} else
		start_y = 0;

	if (y2 > display->r.max.y)
		y2 = display->r.max.y;

	if (y1 >= y2)
		return;

	uint8_t *line = (uint8_t *) ((uintptr_t) (display->base)
			+ y1 * display->bpl);
	y2 -= y1;

	int x1 = x;
	for (uint8_t *p = (uint8_t *) str; *p; p++)
		if (_putc(display, line, &x1, color, font, *p, start_y, y2))
			break;
}

static bool _putc(const Display *display, uint8_t *out, int *x,
		const Color_Rgb *c, Font *font, const uint8_t ch, int start_y,
		size_t len_y)
{
	int x1 = *x;
	if (x1 >= display->r.max.x)
		return true;

	//TODO all glyph has same width?
	int x2 = x1 + font->width;
	if (x2 <= display->r.min.x) {
		*x = x2;
		return false;
	}

	int start_x;
	if (x1 < display->r.min.x) {
		start_x = display->r.min.x - x1;
		x1 = display->r.min.x;
	} else
		start_x = 0;

	if (x2 > display->r.max.x)
		x2 = display->r.max.x;

	if (x1 >= x2)
		return false;

	size_t len_x = x2 - x1;
	size_t skip = display->bpl - len_x * sizeof(Color_Rgb);
	out += x1 * sizeof(Color_Rgb);
	*x = x2;

	size_t bytes = font_bytes(font->width);
	const uint8_t *in = get_font_address(font, ch) + start_y * bytes;
	size_t skip_in = 0;
	for (; start_x >= FONT_BITS; start_x -= FONT_BITS)
		//TODO test with large font
		//TODO divide is faster?
		skip_in++;

	for (; len_y > 0; len_y--) {
		const uint8_t *q = in + skip_in;
		uint8_t pattern = *q++;
		unsigned int mask = 1 << start_x;
		for (size_t j = len_x; j > 0; j--) {
			//TODO color order is BGR?
			if (pattern & mask) {
				out[0] = c[1].b;
				out[1] = c[1].g;
				out[2] = c[1].r;
			} else {
				out[0] = c[0].b;
				out[1] = c[0].g;
				out[2] = c[0].r;
			}

			out += sizeof(Color_Rgb);
			mask <<= 1;
			if (!(mask & ((1 << FONT_BITS) - 1))) {
				//TODO test with large font
				mask = 1;
				pattern = *q++;
			}
		}

		out += skip;
		in += bytes;
	}

	return false;
}

static uint8_t *get_font_address(const Font *font, const uint8_t ch)
{
	uint8_t c = ((ch < font->min_char) || (ch > font->max_char)) ? ' ' : ch;
	return &(font->buf[(c - font->min_char) * font->bytes_per_chr]);
}
#endif
