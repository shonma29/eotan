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
#include <event.h>
#include <nerve/kcall.h>
#include <archfunc.h>
#include <arch/8259a.h>
#include "../../lib/libserv/libserv.h"
#include "hmi.h"
#include "mouse.h"

static hmi_interrupt_t message = { event_mouse, 0 };
static int x;
static int y;
static int buttons = 0;
static int last_sent;


void mouse_handle(const int type, const int d)
{
	buttons = (uint8_t) ((d >> 16) & 0x07);

	int dx = (uint8_t) ((d >> 8) & 0xff);
	//TODO use table by bit mask
	if (d & 0x100000)
		dx |= 0xffffff00;

	//TODO store as static variable
	int width = display->r.max.x - display->r.min.x;

	x += dx;
	if (x < 0)
		x = 0;
	else if (x >= width)
		x = width - 1;

	int dy = (uint8_t) (d & 0xff);
	//TODO use table by bit mask
	if (d & 0x200000)
		dy |= 0xffffff00;

	//TODO store as static variable
	int height = display->r.max.y - display->r.min.y;

	y -= dy;
	if (y < 0)
		y = 0;
	else if (y >= height)
		y = height - 1;

	int data = mouse_encode_data(buttons, x, y);
	if (data != last_sent) {
		message.data = data;

		if (dx || dy)
			message.data |= 0x40000000;

		if (lfq_enqueue(&interrupt_queue, &message))
			//TODO more intelligent skip logic is necessary
			log_warning("interrupt_queue full\n");
		else {
			kcall->ipc_notify(MYPORT, EVENT_INTERRUPT);
			last_sent = data;
		}
	}
}

int mouse_encode_data(const int buttons, const int x, const int y)
{
	return (buttons << 24) | (x << 12) | y;
}

void mouse_show(void)
{
	pointer_put(display, x, y, &(pointer[pointer_select]));
}

void mouse_hide(void)
{
	pointer_restore(display);
}

int mouse_initialize(void)
{
	x = (display->r.max.x - display->r.min.x) / 2 - POINTER_WIDTH;
	y = (display->r.max.y - display->r.min.y) / 2 - POINTER_HEIGHT;
	mouse_show();
	last_sent = mouse_encode_data(buttons, x, y);
	window_focus(last_sent);

	T_CISR pk_cisr = {
		TA_HLNG,
		PIC_IR_VECTOR(ir_mouse),
		PIC_IR_VECTOR(ir_mouse),
		mouse_interrupt
	};
	ER_ID id = create_isr(&pk_cisr);
	if (id < 0) {
		log_err("mouse: bind error=%d\n", id);
		return id;
	}

	int result = enable_interrupt(ir_mouse);
	if (result) {
		log_err("mouse: enable error=%d\n", result);
		destroy_isr(id);
		return result;
	}

	return E_OK;
}
