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
#include <services.h>
#include <core/packets.h>
#include <mpu/io.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include "../../kernel/arch/8042.h"
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "../../kernel/arch/scan2key.h"
#include "../../kernel/arch/key2char.h"
#include "../../lib/libserv/libserv.h"
#include "keyboard.h"

#define LSHIFT 0x01
#define RSHIFT 0x02
#define MASK_SHIFT (LSHIFT | RSHIFT)
#define LCTRL 0x04
#define RCTRL 0x08
#define MASK_CTRL (LCTRL | RCTRL)
#define LALT 0x10
#define RALT 0x20
#define MASK_ALT (LALT | RALT)
#define LWIN 0x40
#define RWIN 0x80
#define MASK_WIN (LWIN | RWIN)
#define CAPS 0x100
#define BREAK 0x200

#define SCAN_BREAK 0x80

static unsigned char state = scan_normal;
static unsigned short modifiers = 0;
static ID keyboard_queue_id;

static void keyboard_process(const int d, const int dummy);


static inline unsigned char is_break(const unsigned char b)
{
	return SCAN_BREAK & b;
}

static inline unsigned char strip_break(const unsigned char b)
{
	return (~SCAN_BREAK) & b;
}

static inline int is_shift(const unsigned short m)
{
	int shift = (m & MASK_SHIFT)? 1:0;
	int caps = (m & CAPS)? 1:0;

	return shift ^ caps;
}

static ER keyboard_interrupt()
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

static void keyboard_process(const int d, const int dummy)
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
