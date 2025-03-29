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
#include <nerve/kcall.h>
#include "hmi.h"
#include "mouse.h"


ER_UINT draw_write(const window_t *wp, const UW size, const char *inbuf,
		const int thread_id)
{
	if (size < DRAW_OP_SIZE)
		return E_PAR;//TODO return POSIX error

	draw_operation_e *op = (draw_operation_e *) inbuf;
	if (*op == draw_op_blit) {
		if (size != DRAW_BLIT_PACKET_SIZE)
			return E_PAR;//TODO return POSIX error

		blit_param_t *param =
				(blit_param_t *) &(inbuf[sizeof(draw_operation_e)]);
		if (param->type != display->type)
			return E_PAR;

		const Frame *s = &(wp->inner);
		rect_normalize(&(param->dest));
		rect_transform(&(param->dest), &(s->r.min));

		Rectangle dest;
		if (rect_intersect(&dest, &((param->dest)), &(s->viewport))) {
			int width = (dest.max.x - dest.min.x) * display->bpp;
			if (param->bpl < width)
				return E_PAR;

			copy_range_t range = {
				(char *) ((uintptr_t) (display->base)
						+ dest.min.y * display->bpl
						+ dest.min.x * display->bpp),
				param->base,
				dest.max.y - dest.min.y,
				width,
				param->bpl
			};
			mouse_hide();

			int result = kcall->skip_copy(thread_id, display->bpl,
					&range, 1);
			mouse_show();
			return (result ? result : size);
		}
	} else
		return E_PAR;//TODO return POSIX error

	return size;
}
