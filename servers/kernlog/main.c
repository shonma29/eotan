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
#include <services.h>
#include <string.h>
#include <set/lf_queue.h>
#include <set/ring.h>
#include <setting.h>
#include <sync.h>
#include <libserv.h>
#include "kernlog.h"

static UB buf[SYSLOG_SIZE];

static ER check_param(const UW start, const UW size);
static size_t lfcopy(UB *outbuf, volatile lfq_t* q, const UW size);
static size_t rcopy(UB *outbuf, ring_t *r, const UW size);
static ER_UINT read(UB *outbuf, const UW dd, const UW start, const UW size);
static ER_UINT write(const UW dd, UB *inbuf, const UW start, const UW size);
static UW execute(devmsg_t *message);
static ER accept(const ID port);
static ER_ID initialize(void);


static ER check_param(const UW start, const UW size)
{
	if (start)	return E_PAR;

	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

static size_t lfcopy(UB *outbuf, volatile lfq_t* q, const UW size)
{
	size_t left;

	for (left = size; left > 0; left--) {
		int w;

		if (lfq_dequeue(q, &w) != QUEUE_OK)
			break;

		*outbuf++ = (UB)(w & 0xff);
	}

	return size - left;
}

static size_t rcopy(UB *outbuf, ring_t *r, const UW size)
{
	size_t left = size;

	while (left > 0) {
		int len = ring_peak_len(r);

		if ((len < 0)
				|| (left < len))
			break;

		left -= len;
		ring_get(r, outbuf);
		outbuf += len;
	}

	return size - left;
}

static ER_UINT read(UB *outbuf, const UW dd, const UW start, const UW size)
{
	ER result = check_param(start, size);

	if (result)	return result;
	if (size != RING_MAX_LEN)	return E_PAR;

	switch (dd) {
	case DESC_KERNLOG:
		return lfcopy(outbuf, (volatile lfq_t*)KERNEL_LOG_ADDR, size);

	case DESC_SYSLOG:
		return rcopy(outbuf, (ring_t*)buf, size);

	default:
		return E_PAR;
	}
}

static ER_UINT write(const UW dd, UB *inbuf, const UW start, const UW size)
{
	ER result = check_param(start, size);

	if (result)	return result;
	if (size > RING_MAX_LEN)	return E_PAR;

	switch (dd) {
	case DESC_SYSLOG:
		break;

	default:
		return E_PAR;
	}

	result = ring_put((ring_t*)buf, inbuf, size);

	if (result < 0)
		return E_SYS;

	return result;
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
		res->body.opn_res.size = RING_MAX_LEN;
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
		result = read(res->body.rea_res.dt,
				req->body.rea_req.dd,
				req->body.rea_req.start,
				req->body.rea_req.size);
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
		result = write(req->body.wri_req.dd,
				req->body.wri_req.dt,
				req->body.wri_req.start,
				req->body.wri_req.size);
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
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	size = kcall->port_accept(port, &rdvno, &(message.req));
	if (size < 0) {
		/*dbg_printf("[KERNLOG] acp_por error=%d\n", size);*/
		return size;
	}

	result = kcall->port_reply(rdvno, &(message.res), execute(&message));
	if (result) {
		/*dbg_printf("[KERNLOG] rpl_rdv error=%d\n", result);*/
	}

	return result;
}

static ER_ID initialize(void)
{
	ER err;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(DDEV_REQ),
			sizeof(DDEV_RES)
	};
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	ring_create(buf, sizeof(buf));

	err = kcall->port_create(PORT_SYSLOG, &pk_cpor);
	if (err) {
		/*dbg_printf("[KERNLOG] cre_por error=%d\n", err);*/

		return err;
	}

	return PORT_SYSLOG;
}

void start(VP_INT exinf)
{
	ER_ID port = initialize();
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	if (port >= 0) {
		/*dbg_printf("[KERNLOG] start port=%d\n", port);*/

		while (accept(port) == E_OK);

		kcall->port_destroy(port);
		/*dbg_printf("[KERNLOG] end\n");*/
	}

	kcall->thread_end_and_destroy();
}
