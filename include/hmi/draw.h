#ifndef _HMI_DRAW_H_
#define _HMI_DRAW_H_
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
#include <stddef.h>
#include <screen.h>

typedef struct {
	int x;
	int y;
} Point;

typedef struct {
	Point min;
	Point max;
} Rectangle;

typedef struct {
	int type1;
	int type2;
	Rectangle r;
	unsigned char buf[0];
} Image;

typedef struct {
	Point point;
	int width;
	int height;
	void *base;
	int bpl;
	char *type;
	Screen *screen;
} Frame;

#define TYPE_B8G8R8 "b8g8r8"

typedef enum draw_operation {
	draw_put = 1,
	draw_pset = 2
} draw_operation_e;

#define DRAW_OPE_SIZE (sizeof(draw_operation_e))

extern Frame *get_screen(void);
extern void put(Frame *, const unsigned int, const size_t, const uint8_t *);
extern void pset(Frame *, const unsigned int, const unsigned int,
		const int);
extern void fill(const Frame *, const int, const int, const int,
		const int, const int);
extern void string(const Frame *, const int, const int, const int,
		Font *, const uint8_t *);
#endif
