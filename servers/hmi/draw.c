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
#include "hmi.h"
#include "mouse.h"

ER_UINT draw_write(const UW size, const char *inbuf)
{
	if (size < DRAW_OP_SIZE)
		return E_PAR;//TODO return POSIX error

	window_t *wp = find_window(2);
	if (!wp)
		return E_NOEXS;//TODO return POSIX error

	draw_operation_e *op = (draw_operation_e *) inbuf;
	if (*op == draw_op_put) {
		if (size < DRAW_PUT_PACKET_SIZE)
			return E_PAR;//TODO return POSIX error

		unsigned int x = ((int *) inbuf)[1 + 0];
		unsigned int y = ((int *) inbuf)[1 + 1];
		mouse_hide();
		draw_put(&(wp->inner), x, y,
				(size - DRAW_PUT_PACKET_SIZE)
						/ sizeof(Color_Rgb),
				(uint8_t *) &(inbuf[DRAW_PUT_PACKET_SIZE]));
		mouse_show();
	} else if (*op == draw_op_pset) {
		if (size != DRAW_PSET_PACKET_SIZE)
			return E_PAR;//TODO return POSIX error

		unsigned int x = ((int *) inbuf)[1 + 0];
		unsigned int y = ((int *) inbuf)[1 + 1];
		int color = ((int *) inbuf)[1 + 2];
		mouse_hide();
		draw_pset(&(wp->inner), x, y, color);
		mouse_show();
	} else
		return E_PAR;//TODO return POSIX error

	return size;
}
