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
#include <archfunc.h>
#include <arch/8259a.h>
#include "../../lib/libserv/libserv.h"
#include "hmi.h"
#include "keyboard.h"

static hmi_interrupt_t message = { event_keyboard, 0 };


void keyboard_handle(const int type, const int data)
{
	message.data = data;

	if (lfq_enqueue(&interrupt_queue, &message))
		//TODO more intelligent skip logic is necessary
		log_warning("interrupt_queue full\n");
	else
		kcall->ipc_notify(MYPORT, EVENT_INTERRUPT);
}

int keyboard_initialize(void)
{
	kbc_initialize();

	T_CISR pk_cisr = {
		TA_HLNG,
		PIC_IR_VECTOR(ir_keyboard),
		PIC_IR_VECTOR(ir_keyboard),
		keyboard_interrupt
	};
	ER_ID id = create_isr(&pk_cisr);
	if (id < 0) {
		log_err("keyboard: bind error=%d\n", id);
		return id;
	}

	W result = enable_interrupt(ir_keyboard);
	if (result) {
		log_err("keyboard: enable error=%d\n", result);
		destroy_isr(id);
		return result;
	}

	return E_OK;
}
