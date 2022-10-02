#ifdef USE_VESA
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
#include <vesa.h>
#include <hmi/draw.h>
#include <mpu/memory.h>

#define FONT_BITS CHAR_BIT

#define INT_MOD_MASK (sizeof(uint32_t) - 1)
#define INT_SHIFT_BITS (2)

static Display display;
#if 0
static inline size_t font_bytes(const size_t width)
{
	return ((width + (CHAR_BIT - 1)) / CHAR_BIT);
}

static bool _putc(const Frame *, uint8_t *, int *, const int,
		Font *, const uint8_t, unsigned int, size_t);
static uint8_t *get_font_address(const Font *, const uint8_t);
#endif

Display *get_display(void)
{
	VesaInfo *v = (VesaInfo *) kern_p2v((void *) VESA_INFO_ADDR);

	if ((v->bits_per_pixel != 24)
			|| (v->red_position != 16)
			|| (v->green_position != 8)
			|| (v->blue_position != 0))
		return NULL;

	display.r.min.x = 0;
	display.r.min.y = 0;
#if USE_MONITOR
	display.r.max.x = v->width / 2;
#else
	display.r.max.x = v->width;
#endif
	display.r.max.y = v->height;
	display.base = (void *) (v->buffer_addr);
	display.bpl = v->bytes_per_line;
	display.bpp = sizeof(Color_Rgb);
	display.type = TYPE_B8G8R8;
	return &display;
}

void draw_put(const Frame *s, const int x, const int y, const size_t size,
		const uint8_t *buf)
{
	int absolute_y = s->r.min.y + y;
	if ((absolute_y < s->viewport.min.y)
			|| (absolute_y >= s->viewport.max.y))
		return;

	int absolute_x = s->r.min.x + x;
	if (absolute_x >= s->viewport.max.x)
		return;

	int offset;
	int rest;
	if (absolute_x < s->viewport.min.x) {
		int skip = s->viewport.min.x - absolute_x;
		offset = skip * sizeof(Color_Rgb);;
		absolute_x = s->viewport.min.x;
		rest = size - skip;
	} else {
		offset = 0;
		rest = size;
	}

	if (absolute_x + rest > s->viewport.max.x)
		rest = s->viewport.max.x - absolute_x;

	uint8_t *w = (uint8_t *) ((uintptr_t) (display.base)
			+ absolute_y * display.bpl
			+ absolute_x * sizeof(Color_Rgb));
	for (int i = 0; i < rest * sizeof(Color_Rgb); i++)
		w[i] = buf[offset + i];
}

void draw_pset(const Frame *s, const int x, const int y, const int color)
{
	int absolute_x = s->r.min.x + x;
	if ((absolute_x < s->viewport.min.x)
			|| (absolute_x >= s->viewport.max.x))
		return;

	int absolute_y = s->r.min.y + y;
	if ((absolute_y < s->viewport.min.y)
			|| (absolute_y >= s->viewport.max.y))
		return;

	uint8_t *r = (uint8_t *) ((uintptr_t) (display.base)
			+ absolute_y * display.bpl
			+ absolute_x * sizeof(Color_Rgb));
	//TODO color order is BGR?
	r[0] = color & 0xff;
	r[1] = (color >> 8) & 0xff;
	r[2] = (color >> 16) & 0xff;
}

void draw_fill(const Frame *s, const int x1, const int y1,
		const int x2, const int y2, const int color)
{
	//TODO normalize param

	int absolute_x1 = s->r.min.x + x1;
	if (absolute_x1 >= s->viewport.max.x)
		return;

	int absolute_y1 = s->r.min.y + y1;
	if (absolute_y1 >= s->viewport.max.y)
		return;

	int absolute_x2 = s->r.min.x + x2;
	if (absolute_x2 <= s->viewport.min.x)
		return;

	int absolute_y2 = s->r.min.y + y2;
	if (absolute_y2 <= s->viewport.min.y)
		return;

	if (absolute_x1 < s->viewport.min.x)
		absolute_x1 = s->viewport.min.x;

	if (absolute_x2 > s->viewport.max.x)
		absolute_x2 = s->viewport.max.x;

	if (absolute_x1 >= absolute_x2)
		return;

	if (absolute_y1 < s->viewport.min.y)
		absolute_y1 = s->viewport.min.y;

	if (absolute_y2 > s->viewport.max.y)
		absolute_y2 = s->viewport.max.y;

	if (absolute_y1 >= absolute_y2)
		return;

	size_t len_x = absolute_x2 - absolute_x1;
	uint32_t buf[sizeof(Color_Rgb)];
	uint8_t *p;
	//TODO color order is BGR?
	uint8_t b = color & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t r = (color >> 16) & 0xff;
	size_t skip = display.bpl - len_x * sizeof(Color_Rgb);
	size_t right = absolute_x2 & INT_MOD_MASK;
	size_t left = absolute_x1 & INT_MOD_MASK;
	if (left) {
		if ((absolute_x1 >> INT_SHIFT_BITS)
				== (absolute_x2 >> INT_SHIFT_BITS)) {
			left = right - left;
			right = 0;
		} else
			left = sizeof(uint32_t) - left;
	}

	size_t middle = (len_x - left - right) >> INT_SHIFT_BITS;
	if (middle) {
		p = (uint8_t *) buf;
		for (size_t i = sizeof(uint32_t); i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}
	}

	p = (uint8_t *) ((uintptr_t) (display.base)
			+ absolute_y1 * display.bpl
			+ absolute_x1 * sizeof(Color_Rgb));
	for (absolute_y2 -= absolute_y1; absolute_y2 > 0; absolute_y2--) {
		for (size_t i = left; i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}

		uint32_t *wword = (uint32_t *) p;
		for (size_t i = middle; i > 0; i--) {
			wword[0] = buf[0];
			wword[1] = buf[1];
			wword[2] = buf[2];
			wword += sizeof(Color_Rgb);
		}

		p = (uint8_t *) wword;
		for (size_t i = right; i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}

		p += skip;
	}
}
#if 0
void string(const Frame *s, const int x, const int y, const int color,
		Font *font, const uint8_t *str)
{
	int y1 = y;
	if (y1 >= (int) (s->height))
		return;

	int y2 = y1 + font->height;
	if (y2 <= 0)
		return;

	unsigned int start_y = 0;
	if (y1 < 0) {
		start_y = -y1;
		y1 = 0;
	}

	if (y2 > s->height)
		y2 = s->height;

	uint8_t *line = (uint8_t *) (s->base) + y1 * s->bpl;
	y2 -= y1;

	int x1 = x;
	for (uint8_t *p = (uint8_t *) str; *p; p++)
		if (_putc(s, line, &x1, color, font, *p, start_y, y2))
			break;
}

static bool _putc(const Frame *s, uint8_t *out, int *x, const int color,
		Font *font, const uint8_t ch, unsigned int start_y,
		size_t len_y)
{
	int x1 = *x;
	if (x1 >= (int) (s->width))
		return true;

	int x2 = x1 + font->width;
	if (x2 <= 0) {
		*x = x2;
		return false;
	}

	unsigned int start_x = 0;
	if (x1 < 0) {
		start_x = -x1;
		x1 = 0;
	}

	if (x2 > s->width)
		x2 = s->width;

	size_t len_x = x2 - x1;
	size_t skip = s->bpl - len_x * sizeof(Color_Rgb);
	out += x1 * sizeof(Color_Rgb);
	*x = x2;

	size_t bytes = font_bytes(font->width);
	const uint8_t *in = get_font_address(font, ch) + start_y * bytes;
	size_t skip_in = 0;
	for (; start_x >= FONT_BITS; start_x -= FONT_BITS)
		//TODO test with large font
		skip_in++;

	//TODO color order is BGR?
	uint8_t b = color & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t r = (color >> 16) & 0xff;
	for (; len_y > 0; len_y--) {
		const uint8_t *q = in + skip_in;
		uint8_t pattern = *q++;
		unsigned int mask = 1 << start_x;
		for (size_t j = len_x; j > 0; j--) {
			if (pattern & mask) {
				out[0] = b;
				out[1] = g;
				out[2] = r;
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
#endif
