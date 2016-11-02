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
#include <keycode.h>
#include <services.h>
#include <core/packets.h>
#include <nerve/kcall.h>
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../lib/libserv/libserv.h"
#include "key2char.h"
#include "keyboard.h"

static unsigned short modifiers = 0;
static ID keyboard_queue_id;

static unsigned int get_modifier(int);
static int get_char(int);
static ER check_param(const UW, const UW);


void keyboard_process(const int d, const int dummy)
{
	//TODO error check
	kcall->queue_send(keyboard_queue_id, d, TMO_POL);
}

static unsigned int get_modifier(int b)
{
	switch (b) {
	case 30:
		b = CAPS;
		break;
	case 44:
		b = LSHIFT;
		break;
	case 57:
		b = RSHIFT;
		break;
	case 58:
		b = LCTRL;
		break;
	case 60:
		b = LALT;
		break;
	case 62:
		b = RALT;
		break;
	case 64:
		b = RCTRL;
		break;
	default:
		b = 0;
		break;
	}

	return b;
}

static int get_char(int b)
{
	int m;

	if (b == 0)
		return -1;

	if (b & BREAK) {
		m = get_modifier(b & ~BREAK);
		if (m)
			modifiers &= ~m;

		return -1;
	}

	m = get_modifier(b);
	if (m) {
		modifiers |= m;
		return -1;
	}

	if (modifiers & MASK_CTRL) {
		b = key2char[key_ctrl][b];
		if (b == 255)
			return -1;

	} else if (is_shift(modifiers)) {
		b = key2char[key_shift][b];
		if (!b)
			return -1;

	} else {
		b = key2char[key_base][b];
		if (!b)
			return -1;
	}

	return b;
}

static ER check_param(const UW start, const UW size)
{
//	if (start)	return E_PAR;

	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

ER_UINT keyboard_read(const UW start, const UW size, UB *outbuf)
{
	ER_UINT result = check_param(start, size);
	size_t i;

	if (result)	return result;

	for (i = 0; i < size;) {
		VP_INT d;

		kcall->queue_receive(keyboard_queue_id, &d);
		d = get_char(d);
		if (d >= 0)
			outbuf[i++] = (UB)(d & 0xff);
	}

	return size;
}

ER keyboard_initialize(void)
{
	W result;
	T_CDTQ pk_cdtq = {
			TA_TFIFO,
			1024 - 1,
			NULL
	};
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)keyboard_interrupt
	};

	kbc_initialize();

	keyboard_queue_id = kcall->queue_create_auto(&pk_cdtq);
	if (keyboard_queue_id < 0) {
		dbg_printf("keyboard: acre_dtq error=%d\n", keyboard_queue_id);

		return keyboard_queue_id;
	}

	result = define_handler(PIC_IR_VECTOR(ir_keyboard), &pk_dinh);
	if (result) {
		dbg_printf("keyboard: interrupt_bind error=%d\n", result);
		kcall->queue_destroy(keyboard_queue_id);
		return result;
	}

	result = enable_interrupt(ir_keyboard);
	if (result) {
		dbg_printf("keyboard: interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		define_handler(PIC_IR_VECTOR(ir_keyboard), &pk_dinh);
		kcall->queue_destroy(keyboard_queue_id);
		return result;
	}

	return E_OK;
}
