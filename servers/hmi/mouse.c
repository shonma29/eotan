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
#include <boot/vesa.h>
#include <core/packets.h>
#include <mpu/io.h>
#include <nerve/icall.h>
#include "../../kernel/arch/8042.h"
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../lib/libserv/libserv.h"

static W width;
static W height;
static W x;
static W y;
static W buttons;

extern void pset(unsigned int x, unsigned int y, int color);

static ER mouse_interrupt(void);
static void mouse_process(const int d, const int dummy);


static ER mouse_interrupt(void)
{
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;

	kbc_wait_to_read();
	b1 = inb(KBC_PORT_DATA);
	b2 = inb(KBC_PORT_DATA);
	b3 = inb(KBC_PORT_DATA);

	//TODO error check
	icall->handle(mouse_process, (b1 << 16) | (b2 << 8) | b3, 0);

	return E_OK;
}

static void mouse_process(const int d, const int dummy)
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
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)mouse_interrupt
	};

	width = v->width;
	height = v->height;
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
