#if 0
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
#include <boot/vesa.h>
#include <mpu/memory.h>

void gfill(const int x1, const int y1, const int x2, const int y2,
		const int col)
{
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);
	unsigned char *line = (unsigned char*)(v->buffer_addr)
			+ y1 * v->bytes_per_line
			+ x1 * 3;
	unsigned char r = (col >> 16) & 0xff;
	unsigned char g = (col >> 8) & 0xff;
	unsigned char b = col & 0xff;
	int i;

	for (i = y1; i < y2; i++) {
		unsigned char *p = line;
		int j;

		for (j = x1; j < x2; j++) {
			*p++ = b;
			*p++ = g;
			*p++ = r;
		}

		line += v->bytes_per_line;
	}
}
#endif
