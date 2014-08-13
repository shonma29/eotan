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
#include <string.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"
#include "../../lib/libserv/bind.h"
#include "ramdisk.h"

static UB buf[BLOCK_NUM * BLOCK_SIZE];

static ER check_param(const UW start, const UW size);
static ER_UINT read(UB *outbuf, const UW start, const UW size);
static ER_UINT write(UB *inbuf, const UW start, const UW size);
static UW execute(devmsg_t *message);
static ER accept(const ID port);
static void load_initrd(void);
static ER_ID initialize(void);


static ER check_param(const UW start, const UW size)
{
	if (start >= sizeof(buf))	return E_PAR;
	if ((start + size) > sizeof(buf))	return E_PAR;

	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

static ER_UINT read(UB *outbuf, const UW start, const UW size)
{
	ER result = check_param(start, size);

	if (result)	return result;

	memcpy(outbuf, &(buf[start]), size);

	return size;
}

static ER_UINT write(UB *inbuf, const UW start, const UW size) {
	ER_UINT result = check_param(start, size);

	if (result)	return result;

	memcpy(&(buf[start]), inbuf, size);

	return size;
}

static UW execute(devmsg_t *message)
{
	DDEV_REQ *req = &(message->req);
	DDEV_RES *res = &(message->res);
	ER_UINT result;
	UW size = 0;

	switch (req->header.msgtyp) {
	case DEV_REA:
		result = read(res->body.rea_res.dt,
				req->body.rea_req.start,
				req->body.rea_req.size);
		res->body.rea_res.dd = req->body.rea_req.dd;
		res->body.rea_res.errcd = (result >= 0)? E_OK:result;
		res->body.rea_res.errinfo = 0;
		res->body.rea_res.a_size = (result >= 0)? result:0;
		size = sizeof(res->body.rea_res)
				- sizeof(res->body.rea_res.dt)
				+ (res->body.rea_res.a_size);
		break;

	case DEV_WRI:
		result = write(req->body.wri_req.dt,
				req->body.wri_req.start,
				req->body.wri_req.size);
		res->body.wri_res.dd = req->body.wri_req.dd;
		res->body.wri_res.errcd = (result >= 0)? E_OK:result;
		res->body.wri_res.errinfo = 0;
		res->body.wri_res.a_size = (result >= 0)? result:0;
		size = sizeof(res->body.wri_res);
		break;

	default:
		break;
	}

	return size + sizeof(res->header);
}

static ER accept(const ID port)
{
	devmsg_t *message;
	RDVNO rdvno;
	ER_UINT size;
	ER result;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	size = kcall->port_accept(port, &rdvno, &message);
	if (size < 0) {
		dbg_printf("[RAMDISK] acp_por error=%d\n", size);
		return size;
	}

	execute(message);
	result = kcall->port_reply(rdvno, &message, 0);
	if (result) {
		dbg_printf("[RAMDISK] rpl_rdv error=%d\n", result);
	}

	return result;
}

static void load_initrd(void)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (info->initrd.size > 0) {
		if (info->initrd.size <= sizeof(buf)) {
			dbg_printf("[RAMDISK] initrd start=%p size=%x\n",
					info->initrd.start, info->initrd.size);
			memcpy(buf, info->initrd.start, info->initrd.size);

		} else {
			dbg_printf("[RAMDISK] initrd too large %x\n",
				info->initrd.size);
		}
	}
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
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	load_initrd();

	port = kcall->port_create_auto(&pk_cpor);
	if (port < 0) {
		dbg_printf("[RAMDISK] acre_por error=%d\n", port);

		return port;
	}

	result = bind_device(get_device_id(DEVICE_MAJOR_RAMDISK, 0),
			(UB*)MYNAME, port, sizeof(buf));
	if (result) {
		dbg_printf("[RAMDISK] bind error=%d\n", result);
		kcall->port_destroy(port);

		return E_SYS;
	}

	return port;
}

void start(VP_INT exinf)
{
	ER_ID port = initialize();
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	if (port >= 0) {
		dbg_printf("[RAMDISK] start port=%d\n", port);

		while (accept(port) == E_OK);

		kcall->port_destroy(port);
		dbg_printf("[RAMDISK] end\n");
	}

	kcall->thread_end_and_destroy();
}
