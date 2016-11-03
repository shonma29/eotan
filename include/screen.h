#ifndef _SCREEN_H_
#define _SCREEN_H_
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
	unsigned int min_char;
	unsigned int max_char;
	unsigned char *buf;
} Font;

typedef struct _screen {
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
} Screen;

#endif
