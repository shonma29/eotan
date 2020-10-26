#ifdef USE_VESA
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
#include <screen.h>
#include <stddef.h>
#include <vesa.h>
#include <hmi/draw.h>
#include <mpu/memory.h>

static Frame screen;


Frame *get_screen(void)
{
	VesaInfo *v = (VesaInfo *) kern_p2v((void *) VESA_INFO_ADDR);

	if ((v->bits_per_pixel != 24)
			|| (v->red_position != 16)
			|| (v->green_position != 8)
			|| (v->blue_position != 0))
		return NULL;

	screen.type1 = IMAGE_TYPE_B8R8;
	screen.type2 = IMAGE_TYPE_G8NL;
	screen.width = v->width;
	screen.height = v->height;
	screen.base = (void *) (v->buffer_addr);
	screen.bpl = v->bytes_per_line;
	return &screen;
}

void put(Frame *s, const unsigned int start, const size_t size,
		const uint8_t *buf)
{
	size_t max = s->height * s->bpl;
	if (start >= max)
		return;

	uint8_t *w = (uint8_t *) (s->base) + start;
	size_t rest = max - start;
	rest = (size > rest) ? rest : size;
	for (int i = 0; i < rest; i++)
		w[i] = buf[i];
}

void pset(Frame *s, const unsigned int x, const unsigned int y,
		const int color)
{
	if ((x < 0)
			|| (x >= s->width)
			|| (y < 0)
			|| (y >= s->height))
		return;

	uint8_t *r = (uint8_t *) (s->base)
			+ y * s->bpl
			+ x * sizeof(Color_Rgb);
	r[0] = color & 0xff;
	r[1] = (color >> 8) & 0xff;
	r[2] = (color >> 16) & 0xff;
}
#endif
