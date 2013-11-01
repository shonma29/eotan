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
#include <console.h>
#include <device.h>
#include <kcall.h>
#include <major.h>
#include <stddef.h>
#include <string.h>
#include <mpu/memory.h>
#include <bind.h>
#include <libserv.h>
#include "../../kernel/setting.h"
#include "hmi.h"
#include "keyboard.h"

#ifdef USE_VESA
#include <boot/vesa.h>

#define MAX_WINDOW (16)

typedef struct {
	UW width;
	UW height;
} Screen;

typedef struct {
	UW left;
	UW top;
	UW width;
	UW height;
	UW cursor_x;
	UW cursor_y;
	UW color;
	UW enabled;
} window_t;

//static window_t window[MAX_WINDOW];

extern Console *getConsole(void);
extern void put(const unsigned int start, const size_t size,
		const unsigned char *buf);
#else
#include <cga.h>
#endif

static Console *cns;
static kcall_t kcall;

static ER check_param(const UW start, const UW size);
static ER_UINT write(const UW dd, const UW start, const UW size,
		const UB *inbuf);
static UW execute(devmsg_t *message);
static ER accept(const ID port);
static ER_ID initialize(void);


static ER check_param(const UW start, const UW size)
{
/*
	if (start)	return E_PAR;
*/
	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

static ER_UINT write(const UW dd, const UW start, const UW size,
		const UB *inbuf)
{
	ER_UINT result = check_param(start, size);
	size_t i;

	if (result)	return result;

	if (dd)
#ifdef USE_VESA
		put(start, size, inbuf);
#else
		;
#endif
	else {
		for (i = 0; i < size; i++)
			cns->putc(inbuf[i]);
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
		res->body.rea_res.dd = req->body.rea_req.dd;
		res->body.rea_res.errcd = E_NOSPT;
		res->body.rea_res.errinfo = 0;
		res->body.rea_res.split = 0;
		res->body.rea_res.a_size = 0;
		size = sizeof(res->body.rea_res)
				- sizeof(res->body.rea_res.dt);
		break;

	case DEV_WRI:
		result = write(req->body.wri_req.dd,
				req->body.wri_req.start,
				req->body.wri_req.size,
				req->body.wri_req.dt);
		res->body.wri_res.dd = req->body.wri_req.dd;
		res->body.wri_res.errcd = (result >= 0)? E_OK:result;
		res->body.wri_res.errinfo = 0;
		res->body.wri_res.a_size = (result >= 0)? result:0;
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

static ER accept(const ID port)
{
	devmsg_t message;
	RDVNO rdvno;
	ER_UINT size;
	ER result;

//	size = acp_por(port, 0xffffffff, &rdvno, &(message.req));
	size = kcall.port_accept(port, &rdvno, &(message.req));
	if (size < 0) {
		dbg_printf("[HMI] acp_por error=%d\n", size);
		return size;
	}

//	result = rpl_rdv(rdvno, &(message.res), execute(&message));
	result = kcall.port_reply(rdvno, &(message.res), execute(&message));
	if (result) {
		dbg_printf("[HMI] rpl_rdv error=%d\n", result);
	}

	return result;
}

static ER_ID initialize(void)
{
	ER_ID port;
	W result;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(DDEV_REQ),
			sizeof(DDEV_RES)
	};

	memcpy(&kcall, (kcall_t*)KCALL_ADDR, sizeof(kcall));

#ifdef USE_VESA
	cns = getConsole();
#else
	cns = getConsole((const UH*)kern_p2v((void*)CGA_VRAM_ADDR));
#endif
	cns->cls();
	cns->locate(0, 0);

//	port = acre_por(&pk_cpor);
	port = kcall.port_create_auto(&pk_cpor);
	if (port < 0) {
		dbg_printf("[HMI] acre_por error=%d\n", port);

		return port;
	}

	result = bind_device(get_device_id(DEVICE_MAJOR_CONSOLE, 0),
			(UB*)MYNAME, port);
	if (result) {
		dbg_printf("[HMI] bind error=%d\n", result);
//		del_por(port);
		kcall.port_destroy(port);

		return E_SYS;
	}

	return port;
}

void start(VP_INT exinf)
{
	ER_ID port = initialize();
	T_CTSK pk = {
		TA_HLNG,
		NULL,
		(FP)keyboard_accept,
		pri_server_middle,
		KERNEL_STACK_SIZE,
		NULL,
		KERNEL_DOMAIN_ID
	};

	if (port >= 0) {
		dbg_printf("[HMI] start port=%d\n", port);

		if (keyboard_initialize() == E_OK) {
			ER_ID t2 = kcall.thread_create_auto(&pk);

			dbg_printf("[HMI]keyboard=%d\n", t2);

			if (t2 > 0)
				kcall.thread_start(t2);
		}

		while (accept(port) == E_OK);

//		del_por(port);
		kcall.port_destroy(port);
		dbg_printf("[HMI] end\n");
	}

	kcall.thread_end_and_destroy();
}
