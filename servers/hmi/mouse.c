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
#include <device.h>
#include <major.h>
#include <boot/vesa.h>
#include <core/packets.h>
#include <mpu/io.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include "../../kernel/arch/8024.h"
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../lib/libserv/bind.h"
#include "../../lib/libserv/libserv.h"

static icall_t *icall = (icall_t*)ICALL_ADDR;
static ID mouse_queue_id;

extern void pset(unsigned int x, unsigned int y, int color);

static ER mouse_interrupt(void);


static ER mouse_interrupt(void)
{
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;

	kbc_wait_to_read();
	b1 = inb(KBC_PORT_DATA);
	b2 = inb(KBC_PORT_DATA);
	b3 = inb(KBC_PORT_DATA);
	icall->queue_send_nowait(mouse_queue_id, (b1 << 16) | (b2 << 8) | b3);

	return E_OK;
}

void mouse_accept(void)
{
	VesaInfo *v = (VesaInfo*)kern_p2v((void*)VESA_INFO_ADDR);
	W width = v->width;
	W height = v->height;
	W x = 0;
	W y = 0;
	W buttons = 0;

	for (;;) {
		int dx, dy;
		W d;

		kcall->queue_receive(mouse_queue_id, &d);
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
}

ER mouse_initialize(void)
{
	W result;
	T_CDTQ pk_cdtq = {
			TA_TFIFO,
			1024 - 1,
			NULL
	};
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)mouse_interrupt
	};

	mouse_queue_id = kcall->queue_create_auto(&pk_cdtq);
	if (mouse_queue_id < 0) {
		dbg_printf("mouse: acre_dtq error=%d\n", mouse_queue_id);

		return mouse_queue_id;
	}

	result = kcall->interrupt_bind(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
	if (result) {
		dbg_printf("mouse: interrupt_bind error=%d\n", result);
		kcall->queue_destroy(mouse_queue_id);
		return result;
	}

	result = kcall->interrupt_enable(ir_mouse);
	if (result) {
		dbg_printf("mouse: interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		kcall->interrupt_bind(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
		kcall->queue_destroy(mouse_queue_id);
		return result;
	}

	return E_OK;
}
