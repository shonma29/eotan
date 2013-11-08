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
#include <kcall.h>
#include <major.h>
#include <core/packets.h>
#include <mpu/io.h>
#include "../../drivers/pcat/8024.h"
#include "../../drivers/pcat/8259a.h"
#include "../../drivers/pcat/scan2key.h"
#include "../../drivers/pcat/key2char.h"
#include "../../lib/libserv/bind.h"
#include "../../lib/libserv/libserv.h"

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

#define SCAN_BREAK 0x80

static unsigned char state = scan_normal;
static unsigned short modifiers = 0;
static kcall_t *kcall = (kcall_t*)KCALL_ADDR;
static ID keyboard_queue_id;
static ID keyboard_port_id;

static unsigned char is_break(const unsigned char b)
{
	return SCAN_BREAK & b;
}

static unsigned char strip_break(const unsigned char b)
{
	return (~SCAN_BREAK) & b;
}

static int is_shift(const unsigned short m)
{
	int shift = (m & MASK_SHIFT)? 1:0;
	int caps = (m & CAPS)? 1:0;

	return shift ^ caps;
}

static void kbc_wait_to_read(void);


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

	if (is_break(b)) {
		b = scan2key[state][strip_break(b)];
		switch (b) {
		case 30:
			modifiers &= ~CAPS;
			return E_OK;
		case 44:
			modifiers &= ~LSHIFT;
			return E_OK;
		case 57:
			modifiers &= ~RSHIFT;
			return E_OK;
		case 58:
			modifiers &= ~LCTRL;
			return E_OK;
		case 60:
			modifiers &= ~LALT;
			return E_OK;
		case 62:
			modifiers &= ~RALT;
			return E_OK;
		case 64:
			modifiers &= ~RCTRL;
			return E_OK;
		default:
			return E_OK;
		}
	}

	b = scan2key[state][b];
	switch (b) {
	case 0:
		return E_OK;
	case 30:
		modifiers |= CAPS;
		return E_OK;
	case 44:
		modifiers |= LSHIFT;
		return E_OK;
	case 57:
		modifiers |= RSHIFT;
		return E_OK;
	case 58:
		modifiers |= LCTRL;
		return E_OK;
	case 60:
		modifiers |= LALT;
		return E_OK;
	case 62:
		modifiers |= RALT;
		return E_OK;
	case 64:
		modifiers |= RCTRL;
		return E_OK;
	default:
		break;
	}

	if (modifiers & MASK_CTRL) {
		b = key2char[key_ctrl][b];
		if (b == 255)
			return E_OK;

	} else if (is_shift(modifiers)) {
		b = key2char[key_shift][b];
		if (!b)
			return E_OK;

	} else {
		b = key2char[key_base][b];
		if (!b)
			return E_OK;
	}

	kcall->queue_send(keyboard_queue_id, b);

	return E_OK;
}

static void kbc_wait_to_read(void)
{
	while (!(inb(KBC_PORT_CMD) & KBC_STATUS_OBF));
}

static ER check_param(const UW start, const UW size)
{
//	if (start)	return E_PAR;

	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

static ER_UINT read(const UW start, const UW size, UB *outbuf)
{
	ER_UINT result = check_param(start, size);
	size_t i;

	if (result)	return result;

	for (i = 0; i < size; i++) {
		VP_INT d;

		kcall->queue_receive(keyboard_queue_id, &d);
		outbuf[i] = (UB)(d & 0xff);
	}

	return size;
}

static UW execute(devmsg_t *message)
{
	DDEV_REQ *req = &(message->req);
	DDEV_RES *res = &(message->res);
	ER_UINT result;
	UW size = 0;

	switch (req->header.msgtyp) {
	case DEV_OPN:
		res->body.opn_res.dd = req->body.opn_req.dd;
		res->body.opn_res.size = DEV_BUF_SIZE;
		res->body.opn_res.errcd = E_OK;
		res->body.opn_res.errinfo = 0;
		size = sizeof(res->body.opn_res);
		break;

	case DEV_CLS:
		res->body.cls_res.dd = req->body.cls_req.dd;
		res->body.cls_res.errcd = E_OK;
		res->body.cls_res.errinfo = 0;
		size = sizeof(res->body.cls_res);
		break;

	case DEV_REA:
		result = read(req->body.rea_req.start, req->body.rea_req.size,
				res->body.rea_res.dt);
		res->body.rea_res.dd = req->body.rea_req.dd;
		res->body.rea_res.errcd = (result >= 0)? E_OK:result;
		res->body.rea_res.errinfo = 0;
		res->body.rea_res.split = 0;
		res->body.rea_res.a_size = (result >= 0)? result:0;
		size = sizeof(res->body.rea_res)
				- sizeof(res->body.rea_res.dt)
				+ (res->body.rea_res.a_size);
		break;

	case DEV_WRI:
		res->body.wri_res.dd = req->body.wri_req.dd;
		res->body.wri_res.errcd = E_NOSPT;
		res->body.wri_res.errinfo = 0;
		res->body.wri_res.a_size = 0;
		size = sizeof(res->body.wri_res);
		break;

	case DEV_CTL:
		res->body.ctl_res.dd = req->body.ctl_req.dd;
		res->body.ctl_res.errcd = E_NOSPT;
		res->body.ctl_res.errinfo = 0;
		size = sizeof(res->body.ctl_res);
		break;

	default:
		break;
	}

	return size + sizeof(res->header);
}

void keyboard_accept(void)
{
	for (;;) {
		devmsg_t message;
		RDVNO rdvno;
		ER_UINT size;
		ER result;

		size = kcall->port_accept(keyboard_port_id, &rdvno,
				&(message.req));
		if (size < 0) {
			dbg_printf("[keyboard] acp_por error=%d\n", size);
			break;
		}

		result = kcall->port_reply(rdvno, &(message.res),
				execute(&message));
		if (result) {
			dbg_printf("[keyboard] rpl_rdv error=%d\n", result);
			break;
		}
	}
}

ER keyboard_initialize(void)
{
	W result;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(DDEV_REQ),
			sizeof(DDEV_RES)
	};
	T_CDTQ pk_cdtq = {
			TA_TFIFO,
			1024 - 1,
			NULL
	};
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)keyboard_interrupt
	};

	keyboard_queue_id = kcall->queue_create_auto(&pk_cdtq);
	if (keyboard_queue_id < 0) {
		dbg_printf("[keyboard] acre_dtq error=%d\n", keyboard_queue_id);

		return keyboard_queue_id;
	}

	result = kcall->interrupt_bind(PIC_IR_VECTOR(ir_keyboard), &pk_dinh);
	if (result) {
		dbg_printf("[keyboard] interrupt_bind error=%d\n", result);
		kcall->queue_destroy(keyboard_queue_id);
		return result;
	}

	result = kcall->interrupt_enable(ir_keyboard);
	if (result) {
		dbg_printf("[keyboard] interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		kcall->interrupt_bind(PIC_IR_VECTOR(ir_keyboard), &pk_dinh);
		kcall->queue_destroy(keyboard_queue_id);
		return result;
	}

	keyboard_port_id = kcall->port_create_auto(&pk_cpor);
	if (keyboard_port_id < 0) {
		dbg_printf("[keyboard] acre_por error=%d\n", keyboard_port_id);

		return result;
	}

	result = bind_device(get_device_id(DEVICE_MAJOR_KEYBOARD, 0),
			(UB*)("keyboard"), keyboard_port_id);
	if (result) {
		dbg_printf("[keyboard] bind error=%d\n", result);
		kcall->port_destroy(keyboard_port_id);
	}

	return E_OK;
}
