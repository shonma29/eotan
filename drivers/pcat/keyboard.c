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
#include <event.h>
#include <keycode.h>
#include <mpu/io.h>
#include <nerve/icall.h>
#include <set/lf_queue.h>
#include "8042.h"
#include "archfunc.h"
#include "scan2key.h"
#include "../../servers/hmi/hmi.h"

extern volatile lfq_t hmi_queue;

static hmi_interrupt_t message = { event_keyboard, 0 };
static unsigned char state = scan_normal;


ER keyboard_interrupt(void)
{
	unsigned char b;

	kbc_wait_to_read();
	b = inb(KBC_PORT_DATA);

	switch (state) {
	case scan_normal:
		switch (b) {
		case 0xe0:
			state = scan_e0;
			return E_OK;
		case 0xe1:
			state = scan_e1;
			return E_OK;
		default:
			break;
		}
		break;
	default:
		state = scan_normal;
		break;
	}

	message.data = (is_break(b) ?
			(BREAK | scan2key[state][strip_break(b)])
			: scan2key[state][b]);
	if (lfq_enqueue(&hmi_queue, &message) == QUEUE_OK)
		//TODO error check
		icall->handle(hmi_handle, 0, 0);

	return E_OK;
}
