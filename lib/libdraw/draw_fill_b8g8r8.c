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
#include <hmi/draw.h>

#define INT_MOD_MASK (sizeof(uint32_t) - 1)
#define INT_SHIFT_BITS (2)


void draw_fill(const Display *display, Rectangle *rect, const int color)
{
	Rectangle src = *rect;
	rect_normalize(&src);

	Rectangle dest;
	if (!rect_intersect(&dest, &src, &(display->r)))
		return;

	size_t len_x = dest.max.x - dest.min.x;
	uint32_t buf[sizeof(Color_Rgb)];
	uint8_t *p;
	//TODO color order is BGR?
	uint8_t b = color & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t r = (color >> 16) & 0xff;
	size_t skip = display->bpl - len_x * sizeof(Color_Rgb);
	size_t right = dest.max.x & INT_MOD_MASK;
	size_t left = dest.min.x & INT_MOD_MASK;
	if (left) {
		if ((dest.min.x >> INT_SHIFT_BITS)
				== (dest.max.x >> INT_SHIFT_BITS)) {
			left = right - left;
			right = 0;
		} else
			left = sizeof(uint32_t) - left;
	}

	size_t middle = (len_x - left - right) >> INT_SHIFT_BITS;
	if (middle) {
		p = (uint8_t *) buf;
		for (size_t i = sizeof(uint32_t); i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}
	}

	p = (uint8_t *) ((uintptr_t) (display->base)
			+ dest.min.y * display->bpl
			+ dest.min.x * sizeof(Color_Rgb));
	for (dest.max.y -= dest.min.y; dest.max.y > 0; dest.max.y--) {
		for (size_t i = left; i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}

		uint32_t *wword = (uint32_t *) p;
		for (size_t i = middle; i > 0; i--) {
			wword[0] = buf[0];
			wword[1] = buf[1];
			wword[2] = buf[2];
			wword += sizeof(Color_Rgb);
		}

		p = (uint8_t *) wword;
		for (size_t i = right; i > 0; i--) {
			p[0] = b;
			p[1] = g;
			p[2] = r;
			p += sizeof(Color_Rgb);
		}

		p += skip;
	}
}
#endif
