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
#include <mpu/io.h>
#include <nerve/icall.h>
#include "8042.h"
#include "archfunc.h"
#include "scan2key.h"
#include "../../servers/hmi/keyboard.h"

static unsigned char state = scan_normal;

static inline unsigned char is_break(const unsigned char b)
{
	return SCAN_BREAK & b;
}

static inline unsigned char strip_break(const unsigned char b)
{
	return (~SCAN_BREAK) & b;
}

ER keyboard_interrupt()
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

	//TODO error check
	icall->handle(keyboard_process,
			is_break(b)?
				(BREAK | scan2key[state][strip_break(b)])
				:scan2key[state][b],
			0);

	return E_OK;
}

unsigned int get_modifier(int b)
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
