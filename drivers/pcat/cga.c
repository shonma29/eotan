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
#include <cga.h>
#include <console.h>
#include <screen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <mpu/io.h>

#define PORT_CRTC 0x03d4
#define CRTC_CURSOR_HIGH 0x0e
#define CRTC_CURSOR_LOW 0x0f

static uint16_t _combine_chr(int, uint8_t);
static void _erase(Screen *, const erase_type_e);
static int _locate(Screen *, const int x, const int);
static int _color(Screen *, const int);
static void _putc(Screen *, const uint8_t);
static void __putc(Screen *, const uint8_t);
static void _newline(Screen *);
static void _cursor(Screen *);
static int _rollup(Screen *, const int);
static void _fill(Screen *, const unsigned int, const unsigned int,
		const unsigned int, const unsigned int);


static Console _cns = {
	_erase, _locate, _color, _putc, _rollup
};


Console *getCgaConsole(Screen *s, const void *base)
{
	s->fgcolor.palet = CGA_DEFAULT_COLOR;
	s->base = base;
	s->wrap = true;
	return &_cns;
}

static uint16_t _combine_chr(int color, uint8_t ch)
{
	return ((color << 8) | ch);
}

static void _erase(Screen *s, const erase_type_e type)
{
	switch (type) {
	case EraseScreenFromCursor:
		_fill(s, s->x, s->y, CGA_COLUMNS, s->y + 1);
		if (s->y + 1 < CGA_COLUMNS)
			_fill(s, 0, s->y + 1, CGA_COLUMNS, CGA_ROWS);
		break;
	case EraseScreenToCursor:
		if (s->y > 0)
			_fill(s, 0, 0, CGA_COLUMNS, s->y);

		_fill(s, 0, s->y, s->x + 1, s->y + 1);
		break;
	case EraseScreenEntire:
		_fill(s, 0, 0, CGA_COLUMNS, CGA_ROWS);
		break;
	case EraseLineFromCursor:
		_fill(s, s->x, s->y, CGA_COLUMNS, s->y + 1);
		break;
	case EraseLineToCursor:
		_fill(s, 0, s->y, s->x + 1, s->y + 1);
		break;
	case EraseLineEntire:
		_fill(s, 0, s->y, CGA_COLUMNS, s->y + 1);
		break;
	}

	_cursor(s);
}

static int _locate(Screen *s, const int x, const int y)
{
	if ((x < 0)
			|| (x >= CGA_COLUMNS)
			|| (y < 0)
			|| (y >= CGA_ROWS))
		return false;

	s->x = x;
	s->y = y;
	s->p = (uint8_t *) ((uint16_t *) (s->base) + y * CGA_COLUMNS + x);
	_cursor(s);
	return true;
}

static int _color(Screen *s, const int color)
{
	if ((color < 0)
			|| (color >= CGA_COLORS))
		return false;

	s->fgcolor.palet = color;
	return true;
}

static void _putc(Screen *s, const uint8_t ch)
{
	switch(ch) {
	case 0x08:
		if (s->x > 0) {
			s->p -= sizeof(uint16_t);

			uint16_t *p = (uint16_t *) (s->p);
			uint16_t *q = p;
			for(size_t rest = CGA_COLUMNS - s->x; rest > 0; rest--) {
				q[0] = q[1];
				q++;
			}

			__putc(s, ' ');
			s->x--;
			s->p = (uint8_t *) p;
		}
		break;
	case '\t':
		{
			size_t len = CONSOLE_TAB_COLUMNS
					- (s->x % CONSOLE_TAB_COLUMNS);
			for (size_t rest = len; rest > 0; rest--)
				__putc(s, ' ');

			if (s->x + len >= CGA_COLUMNS) {
				len = CGA_COLUMNS - 1 - s->x;
				s->x += len;
				s->p += len * sizeof(uint16_t);

				if (s->wrap)
					_newline(s);
			} else {
				s->x += len;
				s->p += len * sizeof(uint16_t);
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

		if (s->x >= CGA_COLUMNS - 1) {
			if (s->wrap)
				_newline(s);
		} else {
			s->x++;
			s->p += sizeof(uint16_t);
		}
		break;
	}

	_cursor(s);
}

static void __putc(Screen *s, const uint8_t ch)
{
	uint16_t *q = (uint16_t *) (s->p);
	*q = _combine_chr(s->fgcolor.palet, ch);
}

static void _newline(Screen *s)
{
	if (s->y >= CGA_ROWS - 1)
		_rollup(s, 1);

	s->x = 0;
	s->y++;
	s->p = (uint8_t *) ((uint16_t *) (s->base) + s->y * CGA_COLUMNS);
}

static void _cursor(Screen *s)
{
	unsigned int pos = s->y * CGA_COLUMNS + s->x;
	outw(PORT_CRTC, (pos & 0xff00) | CRTC_CURSOR_HIGH);
	outw(PORT_CRTC, ((pos & 0xff) << 8) | CRTC_CURSOR_LOW);
}

static int _rollup(Screen *s, const int lines)
{
	if ((lines <= 0)
			|| (lines >= CGA_ROWS))
		return false;

	uint16_t *w = (uint16_t *) (s->base);
	uint16_t *r = w + lines * CGA_COLUMNS;
	for (size_t rest = CGA_COLUMNS * (CGA_ROWS - lines); rest > 0; rest--) {
		*w = *r++;
		w++;
	}

	uint16_t space = _combine_chr(s->fgcolor.palet, ' ');
	for (size_t rest = CGA_COLUMNS * lines; rest > 0; rest--) {
		*w = space;
		w++;
	}

	if ((s->y -= lines) < 0) {
		s->y = s->x = 0;
		s->p = (void *) (s->base);
	} else
		s->p -= lines * CGA_COLUMNS * sizeof(uint16_t);

	return true;
}

static void _fill(Screen *s, const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2)
{
	uint16_t *p = (uint16_t *) (s->base) + y1 * CGA_COLUMNS + x1;
	uint16_t c = _combine_chr(s->fgcolor.palet, ' ');
	for (size_t rest = (y2 - y1 - 1) * CGA_COLUMNS + (x2 - x1); rest > 0;
			rest--) {
		*p = c;
		p++;
	}
}
