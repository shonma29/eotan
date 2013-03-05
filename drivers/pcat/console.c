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
#include <mpu/io.h>

#define PORT_CRTC 0x03d4
#define CRTC_CURSOR_HIGH 0x0e
#define CRTC_CURSOR_LOW 0x0f

#define True 1
#define False 0

static void _cls(void);
static int _locate(const int x, const int y);
static int _color(const int color);
static void _putc(const unsigned char ch);
static void __putc(const unsigned char ch);
static void _newline(void);
static void _cursor(void);
static int _rollup(const int lines);

static CGA_Console _cns = {
	_cls, _locate, _color, _putc, _rollup
};

static struct _screen {
	int x;
	int y;
	int color;
	unsigned short *p;
	unsigned short *base;
} _s;


CGA_Console *getConsole(const unsigned short *base) {
	_s.color = CGA_DEFAULT_COLOR;
	_s.base = (unsigned short*)base;

	return &_cns;
}

static unsigned short _combine_chr(int color, unsigned char ch) {
	return ((color << 8) | ch);
}

static void _cls(void) {
	unsigned short *p = _s.base;
	unsigned int i;
	unsigned short c = _combine_chr(_s.color, ' ');

	_s.x = _s.y = 0;
	_s.p = p;

	for (i = CGA_COLUMNS * CGA_ROWS; i > 0; i--)	*p++ = c;
	_cursor();
}

static int _locate(const int x, const int y) {
	if ((x < 0)
		|| (x >= CGA_COLUMNS)
		|| (y < 0)
		|| (y >= CGA_ROWS))	return False;

	_s.x = x;
	_s.y = y;
	_s.p = _s.base + y * CGA_COLUMNS + x;
	_cursor();

	return True;
}

static int _color(const int color) {
	if ((color < 0)
		|| (color >= CGA_COLORS))	return False;

	_s.color = color;

	return True;
}

static void _putc(const unsigned char ch) {
	switch(ch) {
	case 0x08:
		if (_s.x > 0) {
			unsigned short *p = --_s.p;
			int len = CGA_COLUMNS - _s.x;

			for(; len > 0; len--) {
				_s.p[0] = _s.p[1];
				_s.p++;
			}

			__putc(' ');
			_s.x--;
			_s.p = p;
		}
		break;

	case '\t':
		{
			int len = CGA_TAB_COLUMNS - _s.x % CGA_TAB_COLUMNS;

			for(; len > 0; len--)	__putc(' ');
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

		if (_s.x >= (CGA_COLUMNS - 1))	_newline();
		else {
			_s.x++;
			_s.p++;
		}
		break;
	}

	_cursor();
}

static void __putc(const unsigned char ch) {
	*_s.p = _combine_chr(_s.color, ch);
}

static void _newline(void) {
	if (_s.y >= (CGA_ROWS - 1))	_rollup(1);
	else	_s.y++;

	_s.x = 0;
	_s.p = _s.base + _s.y * CGA_COLUMNS;
}
static void _cursor() {
	unsigned int pos = _s.y * CGA_COLUMNS + _s.x;

	outw(PORT_CRTC, (pos & 0xff00) | CRTC_CURSOR_HIGH);
	outw(PORT_CRTC, ((pos & 0xff) << 8) | CRTC_CURSOR_LOW);
}

static int _rollup(const int lines) {
	unsigned short *w = _s.base;
	unsigned short *r = w + lines * CGA_COLUMNS;
	unsigned int i;
	unsigned short space = _combine_chr(_s.color, ' ');

	if ((lines <= 0)
			|| (lines >= CGA_ROWS))	return False;

	for (i = CGA_COLUMNS * (CGA_ROWS - lines); i > 0; i--)	*w++ = *r++;
	for (i = CGA_COLUMNS * lines; i > 0; i--)	*w++ = space;

	if ((_s.y -= lines) < 0) {
		_s.y = _s.x = 0;
		_s.p = _s.base;
	}
	else	_s.p -= lines * CGA_COLUMNS;

	_cursor();

	return True;
}
