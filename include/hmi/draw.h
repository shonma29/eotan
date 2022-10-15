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
#include <stdint.h>

//TODO BGR order?
typedef struct {
	uint8_t b;
	uint8_t g;
	uint8_t r;
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

typedef struct {
	Rectangle r;
	char *type;
	unsigned char buf[0];
} Image;

typedef struct {
	Rectangle r;
	void *base;
	int bpl;
	int bpp;
	char *type;
} Display;

typedef struct {
	Rectangle r;
	Rectangle viewport;
} Frame;

#define TYPE_B8G8R8 "b8g8r8"

typedef enum draw_operation {
	draw_op_put = 1,
	draw_op_pset = 2
} draw_operation_e;

#define DRAW_OP_SIZE (sizeof(draw_operation_e))
#define DRAW_PUT_PACKET_SIZE (DRAW_OP_SIZE + sizeof(int) * 2)
#define DRAW_PSET_PACKET_SIZE (DRAW_OP_SIZE + sizeof(int) * 3)

#define DRAW_FID (4)

extern Display *get_display(void);
extern void draw_put(const Frame *, const int, const int, const size_t,
		const uint8_t *);
extern void draw_pset(const Frame *, const int, const int, const int);
extern void draw_fill(const Frame *, const int, const int, const int,
		const int, const int);
extern void draw_string(const Frame *, const int, const int, const Color_Rgb *,
		Font *, const uint8_t *);
#endif
