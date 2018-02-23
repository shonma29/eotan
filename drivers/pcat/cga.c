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

static uint16_t _combine_chr(int color, uint8_t ch);
static void _cls(Screen *s);
static int _locate(Screen *s, const int x, const int y);
static int _color(Screen *s, const int color);
static void _putc(Screen *s, const uint8_t ch);
static void __putc(Screen *s, const uint8_t ch);
static void _newline(Screen *s);
static void _cursor(Screen *s);
static int _rollup(Screen *s, const int lines);

static Console _cns = {
	_cls, _locate, _color, _putc, _rollup
};


Console *getCgaConsole(Screen *s, const void *base)
{
	s->fgcolor.palet = CGA_DEFAULT_COLOR;
	s->base = base;

	return &_cns;
}

static uint16_t _combine_chr(int color, uint8_t ch)
{
	return ((color << 8) | ch);
}

static void _cls(Screen *s)
{
	uint16_t *p = (uint16_t*)(s->base);
	uint16_t c = _combine_chr(s->fgcolor.palet, ' ');

	s->x = s->y = 0;
	s->p = (uint8_t*)p;

	for (size_t i = CGA_COLUMNS * CGA_ROWS; i > 0; i--) {
		*p = c;
		p++;
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
	s->p = (uint8_t*)((uint16_t*)(s->base) + y * CGA_COLUMNS + x);
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

			uint16_t *p = (uint16_t*)(s->p);
			uint16_t *q = p;
			for(int len = CGA_COLUMNS - s->x; len > 0; len--) {
				q[0] = q[1];
				q++;
			}

			__putc(s, ' ');
			s->x--;
			s->p = (uint8_t*)p;
		}
		break;

	case '\t':
		{
			int len = CONSOLE_TAB_COLUMNS
					- (s->x % CONSOLE_TAB_COLUMNS);
			for (int i = 0; i < len; i++)
				__putc(s, ' ');

			if (s->x + len >= CGA_COLUMNS)
				_newline(s);
			else {
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
		__putc(s, (ch > ' ')? ch:' ');

		if (s->x >= (CGA_COLUMNS - 1))
			_newline(s);
		else {
			s->x++;
			s->p += sizeof(uint16_t);
		}
		break;
	}

	_cursor(s);
}

static void __putc(Screen *s, const uint8_t ch)
{
	uint16_t *q = (uint16_t*)(s->p);
	*q = _combine_chr(s->fgcolor.palet, ch);
}

static void _newline(Screen *s)
{
	if (s->y >= (CGA_ROWS - 1))
		_rollup(s, 1);

	s->x = 0;
	s->y++;
	s->p = (uint8_t*)((uint16_t*)(s->base) + s->y * CGA_COLUMNS);
}

static void _cursor(Screen *s)
{
	unsigned int pos = s->y * CGA_COLUMNS + s->x;

	outw(PORT_CRTC, (pos & 0xff00) | CRTC_CURSOR_HIGH);
	outw(PORT_CRTC, ((pos & 0xff) << 8) | CRTC_CURSOR_LOW);
}

static int _rollup(Screen *s, const int lines)
{
	uint16_t *w = (uint16_t*)(s->base);
	uint16_t *r = w + lines * CGA_COLUMNS;
	uint16_t space = _combine_chr(s->fgcolor.palet, ' ');

	if ((lines <= 0)
			|| (lines >= CGA_ROWS))
		return false;

	for (size_t i = CGA_COLUMNS * (CGA_ROWS - lines); i > 0; i--) {
		*w = *r++;
		w++;
	}

	for (size_t i = CGA_COLUMNS * lines; i > 0; i--) {
		*w = space;
		w++;
	}

	if ((s->y -= lines) < 0) {
		s->y = s->x = 0;
		s->p = (void*)(s->base);
	}
	else
		s->p -= lines * CGA_COLUMNS * sizeof(uint16_t);

	return true;
}
