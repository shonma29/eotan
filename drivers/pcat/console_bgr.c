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
#include <console.h>
#include <stddef.h>
#include <mpu/memory.h>
#include <nerve/global.h>

static void _erase(Screen *, const erase_type_e);
static int _locate(Screen *, const int, const int);
static void _putc(Screen *, const uint8_t);
static void __putc(Screen *, const uint8_t);
static void _newline(Screen *);
static void _cursor(Screen *);
static int _rollup(Screen *, const int);
static void _fill(Screen *, const unsigned int, const unsigned int,
		const unsigned int, const unsigned int,
		const Color *);
static void _copy_up(Screen *, unsigned int, unsigned int,
		unsigned int, unsigned int, unsigned int);
static void _copy_left(Screen *, unsigned int, unsigned int,
		unsigned int, unsigned int, unsigned int);

static Console _cns = {
	_erase, _locate, _putc
};


Console *getConsole(Screen *s, const Font *default_font)
{
	Display *d = &(sysinfo->display);
	s->x = 0;
	s->y = 0;
	s->width = d->r.max.x;
	s->height = d->r.max.y;
	s->base = (const void *) (d->base);
	s->p = (uint8_t *) (s->base);
	s->bpl = d->bpl;

	s->fgcolor.rgb.b = 0xff;
	s->fgcolor.rgb.g = 0xff;
	s->fgcolor.rgb.r = 0xff;

	s->bgcolor.rgb.b = 0x7f;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 0;

	s->font = *default_font;
	s->chr_width = s->width / s->font.width;
	s->chr_height = s->height / s->font.height;

	s->wrap = true;
	return &_cns;
}

static void _erase(Screen *s, const erase_type_e type)
{
	_cursor(s);

	switch (type) {
	case EraseScreenFromCursor:
		_fill(s, s->x * s->font.width, s->y * s->font.height,
				s->width, (s->y + 1) * s->font.height,
				&(s->bgcolor));
		if ((s->y + 1) * s->font.height < s->height)
			_fill(s, 0, (s->y + 1) * s->font.height, s->width,
					s->height, &(s->bgcolor));
		break;
	case EraseScreenToCursor:
		if (s->y > 0)
			_fill(s, 0, 0, s->width, s->y * s->font.height,
					&(s->bgcolor));

		_fill(s, 0, s->y * s->font.height, (s->x + 1) * s->font.width,
				(s->y + 1) * s->font.height, &(s->bgcolor));
		break;
	case EraseScreenEntire:
		_fill(s, 0, 0, s->width, s->height, &(s->bgcolor));
		break;
	case EraseLineFromCursor:
		_fill(s, s->x * s->font.width, s->y * s->font.height,
				s->width, (s->y + 1) * s->font.height,
				&(s->bgcolor));
		break;
	case EraseLineToCursor:
		_fill(s, 0, s->y * s->font.height, (s->x + 1) * s->font.width,
				(s->y + 1) * s->font.height, &(s->bgcolor));
		break;
	case EraseLineEntire:
		_fill(s, 0, s->y * s->font.height, s->width,
				(s->y + 1) * s->font.height, &(s->bgcolor));
		break;
	}

	_cursor(s);
}

static int _locate(Screen *s, const int x, const int y)
{
	if ((x < 0)
			|| (x >= s->chr_width)
			|| (y < 0)
			|| (y >= s->chr_height))
		return false;

	_cursor(s);

	s->x = x;
	s->y = y;
	s->p = ((uint8_t *) (s->base)
			+ y * s->font.height * s->bpl
			+ x * s->font.width * sizeof(Color_Rgb));

	_cursor(s);
	return true;
}

static void _putc(Screen *s, const uint8_t ch)
{
	_cursor(s);

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
			size_t len = CONSOLE_TAB_COLUMNS
					- (s->x % CONSOLE_TAB_COLUMNS);
			for (size_t rest = len; rest > 0; rest--)
				__putc(s, ' ');

			if (s->x + len >= s->chr_width) {
				len = s->chr_width - 1 - s->x;
				s->x += len;
				s->p += len * s->font.width * sizeof(Color_Rgb);

				if (s->wrap)
					_newline(s);
			} else {
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

		if (s->x >= (s->chr_width - 1)) {
			if (s->wrap)
				_newline(s);
		} else {
			s->x++;
			s->p += s->font.width * sizeof(Color_Rgb);
		}
		break;
	}

	_cursor(s);
}

static void __putc(Screen *s, const uint8_t ch)
{
	uint8_t *line = (uint8_t *) (s->p);
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
	s->p = ((uint8_t *) (s->base) + s->y * s->font.height * s->bpl);
}

static void _cursor(Screen *s)
{
	Color *fg = &(s->fgcolor);
	Color *bg = &(s->bgcolor);
	uint8_t *line = (uint8_t *) (s->p);
	for (unsigned int i = s->font.height; i > 0; i--) {
		uint8_t *p = line;
		for (unsigned int j = s->font.width; j > 0; j--) {
			if ((p[0] == bg->rgb.b)
					&& (p[1] == bg->rgb.g)
					&& (p[2] == bg->rgb.r)) {
				p[0] = fg->rgb.b;
				p[1] = fg->rgb.g;
				p[2] = fg->rgb.r;
			} else if ((p[0] == fg->rgb.b)
					&& (p[1] == fg->rgb.g)
					&& (p[2] == fg->rgb.r)) {
				p[0] = bg->rgb.b;
				p[1] = bg->rgb.g;
				p[2] = bg->rgb.r;
			}

			p += sizeof(Color_Rgb);
		}

		line += s->bpl;
	}
}

static int _rollup(Screen *s, const int lines)
{
	if ((lines <= 0)
			|| (lines >= s->chr_height))
		return false;

	_cursor(s);

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
		s->p = (uint8_t *) (s->base);
	} else
		s->p = ((uint8_t *) (s->p)
				- lines * s->font.height * s->bpl);

	_cursor(s);
	return true;
}

static void _fill(Screen *s, const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2,
		const Color *color)
{
	uint8_t *p = (uint8_t *) (s->base)
			+ y1 * s->bpl
			+ x1 * sizeof(Color_Rgb);
	Color_Rgb c = color->rgb;
	Color_Rgb buf[sizeof(uint32_t)];
	for (unsigned int i = 0; i < sizeof(uint32_t); i++)
		buf[i] = c;

	uint32_t *rword = (uint32_t *) buf;
	size_t len = x2 - x1;
	size_t skip = s->bpl - len * sizeof(Color_Rgb);
	size_t left = (sizeof(uint32_t) - (x1 & (sizeof(uint32_t) - 1)))
			& (sizeof(uint32_t) - 1);
	size_t right = x2 & (sizeof(uint32_t) - 1);
	size_t middle = (len - left - right) >> 2;

	for (unsigned int i = y1; i < y2; i++) {
		for (size_t j = left; j > 0; j--) {
			p[0] = c.b;
			p[1] = c.g;
			p[2] = c.r;
			p += sizeof(Color_Rgb);
		}

		uint32_t *wword = (uint32_t *) p;
		for (size_t j = middle; j > 0; j--) {
			wword[0] = rword[0];
			wword[1] = rword[1];
			wword[2] = rword[2];
			wword += sizeof(Color_Rgb);
		}

		p = (uint8_t *) wword;
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
	uint8_t *w = (uint8_t *) (s->base);
	uint8_t *r = w + height * s->bpl;
	size_t len = (x2 - x1) * sizeof(Color_Rgb);
	size_t left = (sizeof(uint32_t) - (x1 & (sizeof(uint32_t) - 1)))
			& (sizeof(uint32_t) - 1);
	size_t right = x2 & (sizeof(uint32_t) - 1);
	size_t middle = (len - left - right) >> 2;
	size_t skip = s->bpl - len;

	for (size_t i = y2 - y1; i > 0; i--) {
		for (size_t j = left; j > 0; j--) {
			*w = *r;
			r++;
			w++;
		}

		uint32_t *wword = (uint32_t *) w;
		uint32_t *rword = (uint32_t *) r;
		for (size_t j = middle; j > 0; j--) {
			*wword = *rword;
			rword++;
			wword++;
		}

		w = (uint8_t *) wword;
		r = (uint8_t *) rword;
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
	uint8_t *r = (uint8_t *) (s->base)
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
#endif
