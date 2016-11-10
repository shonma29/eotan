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
#include <core.h>
#ifdef USE_VESA
#include <vesa.h>
#else
#include <cga.h>
#endif
#include <core/packets.h>
#include <mpu/memory.h>
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../lib/libserv/libserv.h"
#include "mouse.h"

static W width;
static W height;
static W x;
static W y;
static W buttons;

extern void pset(unsigned int x, unsigned int y, int color);


void mouse_process(const int type, const int d)
{
	int dx, dy;

	buttons = (UB)((d >> 16) & 0x07);

	dx = (UB)((d >> 8) & 0xff);
	if (d & 0x100000)
		dx |= 0xffffff00;
	x += dx;
	if (x < 0)
		x = 0;
	else if (x >= width)
		x = width - 1;

	dy = (UB)(d & 0xff);
	if (d & 0x200000)
		dy |= 0xffffff00;
	y -= dy;
	if (y < 0)
		y = 0;
	else if (y >= height)
		y = height - 1;
#ifdef USE_VESA
	if (buttons & 1)
		pset(x, y, 0xff0000);
	else if (buttons & 2)
		pset(x, y, 0x00ff00);
	else if (buttons & 4)
		pset(x, y, 0xffff00);
#endif
}

ER mouse_initialize(void)
{
	W result;
#ifdef USE_VESA
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);
#endif
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)mouse_interrupt
	};

#ifdef USE_VESA
	width = v->width;
	height = v->height;
#else
	width = CGA_COLUMNS;
	height = CGA_ROWS;
#endif
	x = 0;
	y = 0;
	buttons = 0;

	result = define_handler(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
	if (result) {
		dbg_printf("mouse: interrupt_bind error=%d\n", result);
		return result;
	}

	result = enable_interrupt(ir_mouse);
	if (result) {
		dbg_printf("mouse: interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		define_handler(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
		return result;
	}

	return E_OK;
}
