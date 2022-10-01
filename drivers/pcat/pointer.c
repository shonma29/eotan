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
#include <hmi/draw.h>
#include <hmi/pointer.h>

static uint8_t *pointer_prev_start = NULL;
static size_t pointer_prev_width;
static size_t pointer_prev_height;
//TODO only 24 bit color?
static uint8_t pointer_backup[
		POINTER_WIDTH * sizeof(Color_Rgb) * POINTER_HEIGHT];

static void pointer_save(const Display *, uint8_t *, const size_t,
		const size_t);


void pointer_put(const Display *d, const int x, const int y,
		const pointer_pattern_t *p)
{
	int x1 = x - p->point_x;
	int width = d->r.max.x - d->r.min.x;
	if (x1 >= width)
		return;

	int y1 = y - p->point_y;
	int height = d->r.max.y - d->r.min.y;
	if (y1 >= height)
		return;

	int x2 = x1 + POINTER_WIDTH;
	int y2 = y1 + POINTER_HEIGHT;
	if ((x2 <= 0)
			|| (y2 <= 0))
		return;

	unsigned int start_x = 0;
	unsigned int start_y = 0;
	if (x1 < 0) {
		start_x = -x1;
		x1 = 0;
	}

	if (y1 < 0) {
		start_y = -y1;
		y1 = 0;
	}

	if (x2 > width)
		x2 = width;

	if (y2 > height)
		y2 = height;

	size_t len_x = x2 - x1;
	size_t len_y = y2 - y1;
	size_t skip = d->bpl - len_x * sizeof(Color_Rgb);
	uint8_t *out = (uint8_t *) (d->base)
			+ y1 * d->bpl+ x1 * sizeof(Color_Rgb);
	pointer_save(d, out, len_x, len_y);

	const uint32_t *in = &(p->buf[start_y * POINTER_NUM_PER_LINE]);
	size_t skip_in = 0;
	for (start_x += start_x; start_x >= 32; start_x -= 32)
		skip_in++;

	for (; len_y > 0; len_y--) {
		const uint32_t *q = in + skip_in;
		uint32_t pattern = *q++;
		int shift = 32 - 2 - start_x;

		for (size_t j = len_x; j > 0; j--) {
			char c = (pattern >> shift) & 3;
			if (c) {
				c = ((c & 1) - 1) & 0xff;
				out[0] = c;
				out[1] = c;
				out[2] = c;
			}

			out += sizeof(Color_Rgb);
			shift -= 2;
			if (shift < 0) {
				shift = 30;
				pattern = *q++;
			}
		}

		out += skip;
		in += POINTER_NUM_PER_LINE;
	}
}

static void pointer_save(const Display *d, uint8_t *start, const size_t width,
		const size_t height)
{
	pointer_prev_start = start;
	pointer_prev_width = width * sizeof(Color_Rgb);
	pointer_prev_height = height;

	uint8_t *dest = pointer_backup;
	uint8_t *line = pointer_prev_start;
	for (size_t i = pointer_prev_height; i > 0; i--) {
		for (int j = 0; j < pointer_prev_width; j++)
			dest[j] = line[j];

		dest += pointer_prev_width;
		line += d->bpl;
	}
}

void pointer_restore(const Display *d)
{
	if (!pointer_prev_start)
		return;

	uint8_t *source = pointer_backup;
	uint8_t *line = pointer_prev_start;
	for (size_t i = pointer_prev_height; i > 0; i--) {
		for (int j = 0; j < pointer_prev_width; j++)
			line[j] = source[j];

		source += pointer_prev_width;
		line += d->bpl;
	}

	pointer_prev_start = NULL;
}
#endif
