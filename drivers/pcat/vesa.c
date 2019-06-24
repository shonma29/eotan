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
#include <console.h>
#include <screen.h>
#include <stdbool.h>
#include <stddef.h>
#include <vesa.h>
#include <mpu/memory.h>

static void _cls(Screen *s);
static int _locate(Screen *s, const int x, const int y);
static int _color(Screen *s, const int color);
static void _putc(Screen *s, const uint8_t ch);
static void __putc(Screen *s, const uint8_t ch);
static void _newline(Screen *s);
static void _cursor(Screen *s);
static int _rollup(Screen *s, const int lines);
static void _fill(Screen *s, const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2,
		const Color *color);
static void _copy_up(Screen *s, unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int height);
static void _copy_left(Screen *s, unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int width);

static Console _cns = {
	_cls, _locate, _color, _putc, _rollup
};


Console *getVesaConsole(Screen *s, const Font *default_font)
{
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);

	if ((v->bits_per_pixel != 24)
			|| (v->red_position != 16)
			|| (v->green_position != 8)
			|| (v->blue_position != 0)) {
		return NULL;
	}

	s->x = 0;
	s->y = 0;
	s->width = v->width;
	s->height = v->height;
	s->base = (const void*)(v->buffer_addr);
	s->p = (uint8_t*)(s->base);
	s->bpl = v->bytes_per_line;

	s->fgcolor.rgb.b = 0xff;
	s->fgcolor.rgb.g = 0xff;
	s->fgcolor.rgb.r = 0xff;

	s->bgcolor.rgb.b = 0x7f;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 0;

	s->font = *default_font;
	s->chr_width = s->width / s->font.width;
	s->chr_height = s->height / s->font.height;

	return &_cns;
}

static void _cls(Screen *s)
{
	s->x = s->y = 0;
	s->p = (uint8_t*)(s->base);

	_fill(s, 0, 0, s->width, s->height, &(s->bgcolor));
	_cursor(s);
}

static int _locate(Screen *s, const int x, const int y)
{
	if ((x < 0)
			|| (x >= s->chr_width)
			|| (y < 0)
			|| (y >= s->chr_height))
		return false;

	s->x = x;
	s->y = y;
	s->p = ((uint8_t*)(s->base)
			+ y * s->font.height * s->bpl
			+ x * s->font.width * sizeof(Color_Rgb));
	_cursor(s);

	return true;
}

static int _color(Screen *s, const int color)
{
	if ((color < 0)
			|| (color > MAX_COLOR))
		return false;

	s->fgcolor.rgb.r = (color >> 16) & 0xff;
	s->fgcolor.rgb.g = (color >> 8) & 0xff;
	s->fgcolor.rgb.b = color & 0xff;

	return true;
}

static void _putc(Screen *s, const uint8_t ch)
{
	switch(ch) {
	case 0x08:
		if (s->x > 0) {
			_copy_left(s, s->x * s->font.width,
					s->y * s->font.height,
					s->chr_width * s->font.width,
					(s->y + 1) * s->font.height,
					s->font.width);
			_fill(s, (s->chr_width - 1) * s->font.width,
					s->y * s->font.height,
					s->chr_width * s->font.width,
					(s->y + 1) * s->font.height,
					&(s->bgcolor));
			s->x--;
			s->p -= s->font.width * sizeof(Color_Rgb);
		}
		break;

	case '\t':
		{
			int len = CONSOLE_TAB_COLUMNS
					- (s->x % CONSOLE_TAB_COLUMNS);
			for (int i = 0; i < len; i++)
				__putc(s, ' ');

			if (s->x + len >= s->chr_width)
				_newline(s);
			else {
				s->x += len;
				s->p += len * s->font.width * sizeof(Color_Rgb);
			}
		}
		break;

	case '\n':
		_newline(s);
		break;

	case '\r':
		break;

	case '\x7f':
		break;

	default:
		__putc(s, (ch > ' ') ? ch : ' ');

		if (s->x >= (s->chr_width - 1))
			_newline(s);
		else {
			s->x++;
			s->p += s->font.width * sizeof(Color_Rgb);
		}
		break;
	}

	_cursor(s);
}

static void __putc(Screen *s, const uint8_t ch)
{
	uint8_t *line = (uint8_t*)(s->p);
	uint8_t c = ((ch < s->font.min_char) || (ch > s->font.max_char)) ?
			' ' : ch;
	uint8_t *q = &(s->font.buf[(c - s->font.min_char)
			* s->font.bytes_per_chr]);

	for (size_t i = s->font.height; i > 0; i--) {
		uint8_t *p = line;
		uint8_t b = 1;

		for (size_t j = s->font.width; j > 0; j--) {
			Color *color;

			if (!b) {
				b = 1;
				q++;
			}

			color = (*q & b) ? &(s->fgcolor) : &(s->bgcolor);
			p[0] = color->rgb.b;
			p[1] = color->rgb.g;
			p[2] = color->rgb.r;
			p += sizeof(Color_Rgb);
			b <<= 1;
		}

		line += s->bpl;
		q++;
	}
}

static void _newline(Screen *s)
{
	if (s->y >= (s->chr_height - 1))
		_rollup(s, 1);

	s->x = 0;
	s->y++;
	s->p = ((uint8_t*)(s->base)
			+ s->y * s->font.height * s->bpl);
}

static void _cursor(Screen *s)
{
}

static int _rollup(Screen *s, const int lines)
{
	if ((lines <= 0)
			|| (lines >= s->chr_height))
		return false;

	_copy_up(s, 0,
			lines * s->font.height,
			s->chr_width * s->font.width,
			s->chr_height * s->font.height,
			s->font.height);
	_fill(s, 0,
			(s->chr_height - lines) * s->font.height,
			s->chr_width * s->font.width,
			s->chr_height * s->font.height,
			&(s->bgcolor));

	if ((s->y -= lines) < 0) {
		s->y = s->x = 0;
		s->p = (uint8_t*)(s->base);
	} else
		s->p = ((uint8_t*)(s->p)
				- lines * s->font.height * s->bpl);

	return true;
}

static void _fill(Screen *s, const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2,
		const Color *color)
{
	uint8_t *p = (uint8_t*)(s->base)
			+ y1 * s->bpl
			+ x1 * sizeof(Color_Rgb);
	Color_Rgb c = color->rgb;
	Color_Rgb buf[sizeof(uint32_t)];
	for (unsigned int i = 0; i < sizeof(uint32_t); i++)
		buf[i] = c;

	uint32_t *rword = (uint32_t*)buf;
	size_t len = x2 - x1;
	size_t skip = s->bpl - len * sizeof(Color_Rgb);
	size_t left = x1 % sizeof(uint32_t);
	if (left)
		left = sizeof(uint32_t) - left;

	size_t right = x2 % sizeof(uint32_t);
	size_t middle = (len - left - right) / sizeof(uint32_t);

	for (unsigned int i = y1; i < y2; i++) {
		for (size_t j = left; j > 0; j--) {
			p[0] = c.b;
			p[1] = c.g;
			p[2] = c.r;
			p += sizeof(Color_Rgb);
		}

		uint32_t *wword = (uint32_t*)p;
		for (size_t j = middle; j > 0; j--) {
			wword[0] = rword[0];
			wword[1] = rword[1];
			wword[2] = rword[2];
			wword += sizeof(Color_Rgb);
		}

		p = (uint8_t*)wword;
		for (size_t j = right; j > 0; j--) {
			p[0] = c.b;
			p[1] = c.g;
			p[2] = c.r;
			p += sizeof(Color_Rgb);
		}

		p += skip;
	}
}

static void _copy_up(Screen *s, unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int height)
{
	uint8_t *w = (uint8_t*)(s->base);
	uint8_t *r = w + height * s->bpl;
	size_t len = (x2 - x1) * sizeof(Color_Rgb);
	size_t left = x1 % sizeof(uint32_t);
	if (left)
		left = sizeof(uint32_t) - left;

	size_t right = x2 % sizeof(uint32_t);
	size_t middle = (len - left - right) / sizeof(uint32_t);
	size_t skip = s->bpl - len;

	for (size_t i = y2 - y1; i > 0; i--) {
		for (size_t j = left; j > 0; j--) {
			*w = *r;
			r++;
			w++;
		}

		uint32_t *wword = (uint32_t*)w;
		uint32_t *rword = (uint32_t*)r;
		for (size_t j = middle; j > 0; j--) {
			*wword = *rword;
			rword++;
			wword++;
		}

		w = (uint8_t*)wword;
		r = (uint8_t*)rword;
		for (size_t j = right; j > 0; j--) {
			*w = *r;
			r++;
			w++;
		}

		w += skip;
		r += skip;
	}
}

static void _copy_left(Screen *s, unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int width)
{
	uint8_t *r = (uint8_t*)(s->base)
			+ y1 * s->bpl
			+ x1 * sizeof(Color_Rgb);
	uint8_t *w = r - width * sizeof(Color_Rgb);
	size_t len = (x2 - x1) * sizeof(Color_Rgb);
	size_t i;
	size_t skip = s->bpl - len;

	for (i = y2 - y1; i > 0; i--) {
		for(size_t j = len; j > 0; j--) {
			*w = *r;
			r++;
			w++;
		}

		w += skip;
		r += skip;
	}
}

void put(Screen *s, const unsigned int start, const size_t size,
		const int8_t *buf)
{
	int8_t *w = (int8_t*)(s->base) + start;

	for (size_t i = 0; i < size; i++)
		w[i] = buf[i];
}

void pset(Screen *s, unsigned int x, unsigned int y, int color)
{
	uint8_t *r = (uint8_t*)(s->base)
			+ y * s->bpl
			+ x * sizeof(Color_Rgb);

	r[0] = color & 0xff;
	r[1] = (color >> 8) & 0xff;
	r[2] = (color >> 16) & 0xff;
}
#endif
