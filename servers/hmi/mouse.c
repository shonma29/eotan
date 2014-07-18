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
static kcall_t *kcall = (kcall_t*)KCALL_ADDR;
static ID mouse_queue_id;
static ID mouse_port_id;


static ER mouse_interrupt()
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

static ER check_param(const UW start, const UW size)
{
//	if (start)	return E_PAR;

	if (size > DEV_BUF_SIZE)	return E_PAR;
	else if (size < sizeof(W))	return E_PAR;

	return E_OK;
}

static ER_UINT read(const UW start, const UW size, UB *outbuf)
{
	ER_UINT result = check_param(start, size);
	size_t i;

	if (result)	return result;

	for (i = 0; i < size;) {
		W d;

		kcall->queue_receive(mouse_queue_id, &d);
		if (d >= 0) {
			outbuf[i++] = (UB)(d & 0xff);
			outbuf[i++] = (UB)((d >> 8) & 0xff);
			outbuf[i++] = (UB)((d >> 16) & 0xff);
			outbuf[i++] = (UB)((d >> 24) & 0xff);
			break;
		}
	}

	return i;
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

void mouse_accept(void)
{
	for (;;) {
		devmsg_t message;
		RDVNO rdvno;
		ER_UINT size;
		ER result;

		size = kcall->port_accept(mouse_port_id, &rdvno,
				&(message.req));
		if (size < 0) {
			dbg_printf("[mouse] acp_por error=%d\n", size);
			break;
		}

		result = kcall->port_reply(rdvno, &(message.res),
				execute(&message));
		if (result) {
			dbg_printf("[mouse] rpl_rdv error=%d\n", result);
			break;
		}
	}
}

ER mouse_initialize(void)
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
		(FP)mouse_interrupt
	};

	mouse_queue_id = kcall->queue_create_auto(&pk_cdtq);
	if (mouse_queue_id < 0) {
		dbg_printf("[mouse] acre_dtq error=%d\n", mouse_queue_id);

		return mouse_queue_id;
	}

	result = kcall->interrupt_bind(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
	if (result) {
		dbg_printf("[mouse] interrupt_bind error=%d\n", result);
		kcall->queue_destroy(mouse_queue_id);
		return result;
	}

	result = kcall->interrupt_enable(ir_mouse);
	if (result) {
		dbg_printf("[mouse] interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		kcall->interrupt_bind(PIC_IR_VECTOR(ir_mouse), &pk_dinh);
		kcall->queue_destroy(mouse_queue_id);
		return result;
	}

	mouse_port_id = kcall->port_create_auto(&pk_cpor);
	if (mouse_port_id < 0) {
		dbg_printf("[mouse] acre_por error=%d\n", mouse_port_id);

		return result;
	}

	result = bind_device(get_device_id(DEVICE_MAJOR_MOUSE, 0),
			(UB*)("mouse"), mouse_port_id);
	if (result) {
		dbg_printf("[mouse] bind error=%d\n", result);
		kcall->port_destroy(mouse_port_id);
	}

	return E_OK;
}
