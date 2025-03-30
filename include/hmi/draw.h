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
#include <features.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
#ifdef PIXEL_BGR
	uint8_t b;
	uint8_t g;
	uint8_t r;
#else
	uint8_t r;
	uint8_t g;
	uint8_t b;
#endif
} Color_Rgb;

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int bytes_per_chr;
	unsigned int min_char;
	unsigned int max_char;
	uint8_t *buf;
} Font;

typedef struct Point {
	int x;
	int y;
} Point;

typedef struct Rectangle {
	Point min;
	Point max;
} Rectangle;

typedef enum {
	B8G8R8 = 1,
	B8G8R8A8 = 2
} display_type_e;

typedef struct {
	Rectangle r;
	void *base;
	int bpl;
	int bpp;
	display_type_e type;
} Display;

typedef struct {
	Rectangle r;
	Rectangle viewport;
} Frame;

typedef struct {
	Rectangle dest;
	void *base;
	size_t bpl;
	display_type_e type;
} blit_param_t;

typedef enum draw_operation {
	draw_op_blit = 1
} draw_operation_e;

#define DRAW_OP_SIZE (sizeof(draw_operation_e))
#define DRAW_BLIT_PACKET_SIZE (DRAW_OP_SIZE + sizeof(blit_param_t))

extern void draw_fill(const Frame *, Rectangle *, const int);
extern void draw_string(const Display *, const int, const int,
		const Color_Rgb *, Font *, const char *);

extern void rect_normalize(Rectangle *);
extern void rect_transform(Rectangle *, const Point *);
extern bool rect_intersect(Rectangle *, const Rectangle *, const Rectangle *);

#endif
