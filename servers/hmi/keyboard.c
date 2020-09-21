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
#include <keycode.h>
#include <core/packets.h>
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../lib/libserv/libserv.h"
#include "key2char.h"
#include "keyboard.h"

static unsigned short modifiers = 0;

static unsigned int get_modifier(int);


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

ER_UINT get_char(int b)
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

ER keyboard_initialize(void)
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
