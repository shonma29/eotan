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
#include <limits.h>
#include <console.h>
#include <stddef.h>
#include <boot/vesa.h>
#include <mpu/memory.h>
#include "font.h"

#define True 1
#define False 0

#define MAX_COLOR 0x00ffffff

typedef struct {
	unsigned char b;
	unsigned char g;
	unsigned char r;
} Color;

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int bytes_per_chr;
	unsigned char *buf;
} Font;

static struct _screen {
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	Color *p;
	Color *base;
	unsigned int bpl;
	Color fgcolor;
	Color bgcolor;
	Font font;
	unsigned int chr_width;
	unsigned int chr_height;
} _s;

static void _cls(void);
static int _locate(const int x, const int y);
static int _color(const int color);
static void _putc(const unsigned char ch);
static void __putc(const unsigned char ch);
static void _newline(void);
static void _cursor(void);
static int _rollup(const int lines);
static void _fill(const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2,
		const Color *color);
static void _copy_up(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int height);
static void _copy_left(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int width);

static Console _cns = {
	_cls, _locate, _color, _putc, _rollup
};


Console *getConsole()
{
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);

	if ((v->bits_per_pixel != 24)
			|| (v->red_position != 16)
			|| (v->green_position != 8)
			|| (v->blue_position != 0)) {
		return NULL;
	}

	_s.x = 0;
	_s.y = 0;
	_s.width = v->width;
	_s.height = v->height;
	_s.p = (Color*)(v->buffer_addr);
	_s.base = _s.p;
	_s.bpl = v->bytes_per_line;

	_s.fgcolor.b = 0xff;
	_s.fgcolor.g = 0xff;
	_s.fgcolor.r = 0xff;

	_s.bgcolor.b = 0x7f;
	_s.bgcolor.g = 0;
	_s.bgcolor.r = 0;

	_s.font.width = CHR_WIDTH;
	_s.font.height = CHR_HEIGHT;
	_s.font.bytes_per_chr = ((CHR_WIDTH + CHAR_BIT - 1) / CHAR_BIT)
			* CHR_HEIGHT;
	_s.font.buf = hmi_default_font;
	_s.chr_width = v->width / _s.font.width;
	_s.chr_height = v->height / _s.font.height;

	return &_cns;
}

static void _cls(void) {
	_s.x = _s.y = 0;
	_s.p = _s.base;

	_fill(0, 0, _s.width, _s.height, &(_s.bgcolor));
	_cursor();
}

static int _locate(const int x, const int y)
{
	if ((x < 0)
			|| (x >= _s.chr_width)
			|| (y < 0)
			|| (y >= _s.chr_height))
		return False;

	_s.x = x;
	_s.y = y;
	_s.p = (Color*)((unsigned char*)(_s.base)
			+ y * _s.font.height * _s.bpl
			+ x * _s.font.width * sizeof(Color));
	_cursor();

	return True;
}

static int _color(const int color)
{
	if ((color < 0)
			|| (color > MAX_COLOR))
		return False;

	_s.fgcolor.r = (color >> 16) & 0xff;
	_s.fgcolor.g = (color >> 8) & 0xff;
	_s.fgcolor.b = color & 0xff;

	return True;
}

static void _putc(const unsigned char ch)
{
	switch(ch) {
	case 0x08:
		if (_s.x > 0) {
			_copy_left(_s.x * _s.font.width,
					_s.y * _s.font.height,
					_s.chr_width * _s.font.width,
					(_s.y  + 1) * _s.font.height,
					_s.font.width);
			_fill((_s.chr_width - 1) * _s.font.width,
					_s.y * _s.font.height,
					_s.chr_width * _s.font.width,
					(_s.y  + 1) * _s.font.height,
					&(_s.bgcolor));
			_s.x--;
			_s.p -= _s.font.width;
		}
		break;

	case '\t':
		{
			int len = CONSOLE_TAB_COLUMNS
					- (_s.x % CONSOLE_TAB_COLUMNS);
			int i;

			for (i = 0; i < len; i++)
				__putc(' ');

			if (_s.x + len >= _s.chr_width)
				_newline();
			else {
				_s.x += len;
				_s.p += len * _s.font.width;
			}
		}
		break;

	case '\n':
		_newline();
		break;

	case '\r':
		break;

	case '\x7f':
		break;

	default:
		__putc((ch > ' ')? ch:' ');

		if (_s.x >= (_s.chr_width - 1))
			_newline();
		else {
			_s.x++;
			_s.p += _s.font.width;
		}
		break;
	}

	_cursor();
}

static void __putc(const unsigned char ch)
{
	unsigned char *line = (unsigned char*)(_s.p);
	unsigned char c = ((ch < MIN_CHAR) || (ch > MAX_CHAR))? ' ':ch;
	unsigned char *q = &(_s.font.buf[(c - MIN_CHAR)
			* _s.font.bytes_per_chr]);
	size_t i;

	for (i = _s.font.height; i > 0; i--) {
		unsigned char *p = line;
		unsigned char b = 1;
		size_t j;

		for (j = _s.font.width; j > 0; j--) {
			Color *color;

			if (!b) {
				b = 1;
				q++;
			}

			color = (*q & b)?
					&(_s.fgcolor):&(_s.bgcolor);
			*p++ = color->b;
			*p++ = color->g;
			*p++ = color->r;
			b <<= 1;
		}

		line += _s.bpl;
		q++;
	}
}

static void _newline(void) {
	if (_s.y >= (_s.chr_height - 1))
		_rollup(1);

	_s.x = 0;
	_s.y++;
	_s.p = (Color*)((unsigned char*)(_s.base)
			+ _s.y * _s.font.height * _s.bpl);
}

static void _cursor()
{
}

static int _rollup(const int lines)
{
	if ((lines <= 0)
			|| (lines >= _s.chr_height))
		return False;

	_copy_up(0,
			lines * _s.font.height,
			_s.chr_width * _s.font.width, 
			_s.chr_height * _s.font.height,
			_s.font.height);
	_fill(0,
			(_s.chr_height - lines) * _s.font.height,
			_s.chr_width * _s.font.width,
			_s.chr_height * _s.font.height,
			&(_s.bgcolor));

	if ((_s.y -= lines) < 0) {
		_s.y = _s.x = 0;
		_s.p = _s.base;
	} else
		_s.p = (Color*)((unsigned char*)(_s.p)
				- lines * _s.font.height * _s.bpl);

	return True;
}

static void _fill(const unsigned int x1, const unsigned int y1,
		const unsigned int x2, const unsigned int y2,
		const Color *color)
{
	unsigned char *line = (unsigned char*)(_s.base)
			+ y1 * _s.bpl
			+ x1 * sizeof(Color);
	Color c = *color;
	size_t i;

	for (i = y1; i < y2; i++) {
		unsigned char *p = line;
		size_t j;

		for (j = x1; j < x2; j++) {
			*p++ = c.b;
			*p++ = c.g;
			*p++ = c.r;
		}

		line += _s.bpl;
	}
}

static void _copy_up(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int height)
{
	unsigned char *line = (unsigned char*)(_s.base);
	size_t offset = height * _s.bpl;
	size_t len = (x2 - x1) * sizeof(Color);
	size_t i;

	for (i = y2 - y1; i > 0; i--) {
		unsigned char *w = line;
		unsigned char *r = line + offset;
		size_t j;

		for (j = len; j > 0; j--)
			*w++ = *r++;

		line += _s.bpl;
	}
}

static void _copy_left(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int width)
{
	unsigned char *line = (unsigned char*)(_s.base)
			+ y1 * _s.bpl
			+ x1 * sizeof(Color);
	size_t len = (x2 - x1) * sizeof(Color);
	size_t i;

	for (i = y2 - y1; i > 0; i--) {
		unsigned char *w = line - width * sizeof(Color);
		unsigned char *r = line;
		size_t j;

		for(j = len; j > 0; j--)
			*w++ = *r++;

		line += _s.bpl;
	}
}

void put(const unsigned int start, const size_t size,
		const unsigned char *buf)
{
	unsigned char *w = (unsigned char*)(_s.base) + start;
	size_t i;

	for (i = 0; i < size; i++)
		w[i] = buf[i];
}
