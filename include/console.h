#ifndef _CONSOLE_H_
#define _CONSOLE_H_
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
#include <stdbool.h>
#include <stdint.h>
#include <hmi/draw.h>

#define MAX_COLOR 0x00ffffff

#define CONSOLE_TAB_COLUMNS (8)

typedef enum {
	EraseScreenFromCursor,
	EraseScreenToCursor,
	EraseScreenEntire,
	EraseLineFromCursor,
	EraseLineToCursor,
	EraseLineEntire
} erase_type_e;

typedef union {
	uint32_t palet;
	Color_Rgb rgb;
} Color;

typedef struct _screen {
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	uint8_t *p;
	const uint8_t *base;
	unsigned int bpl;
	Color fgcolor;
	Color bgcolor;
	Font font;
	unsigned int chr_width;
	unsigned int chr_height;
	bool wrap;
} Screen;

typedef struct {
	void (*erase)(Screen *, const erase_type_e);
	int (*locate)(Screen *, const int, const int);
	void (*putc)(Screen *, const unsigned char);
} Console;

#ifndef USE_UEFI
extern Console *getVesaConsole(Screen *, const Font *);
#endif

extern Font default_font;

#endif
