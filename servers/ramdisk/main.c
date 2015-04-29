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
	ER_UINT result;
	UW size = 0;

	switch (message->Rread.operation) {
	case operation_read:
		result = read(message->Rread.data,
				message->Rread.offset,
				message->Rread.length);
		message->Tread.length = result;
		size = sizeof(message->Tread);
		break;

	case operation_write:
		result = write(message->Rwrite.data,
				message->Rwrite.offset,
				message->Rwrite.length);
		message->Twrite.length = result;
		size = sizeof(message->Twrite);
		break;

	default:
		break;
	}

	return size;
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
		dbg_printf(MYNAME ": acp_por error=%d\n", size);
		return size;
	}

	execute(message);
	result = kcall->port_reply(rdvno, &message, 0);
	if (result) {
		dbg_printf(MYNAME ": rpl_rdv error=%d\n", result);
	}

	return result;
}

static void load_initrd(void)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (info->initrd.size > 0) {
		if (info->initrd.size <= sizeof(buf)) {
			dbg_printf(MYNAME ": initrd start=%p size=%x\n",
					info->initrd.start, info->initrd.size);
			memcpy(buf, info->initrd.start, info->initrd.size);

		} else {
			dbg_printf(MYNAME ": initrd too large %x\n",
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
			sizeof(devmsg_t),
			sizeof(devmsg_t)
	};
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	load_initrd();

	port = kcall->port_create_auto(&pk_cpor);
	if (port < 0) {
		dbg_printf(MYNAME ": acre_por error=%d\n", port);

		return port;
	}

	result = bind_device(get_device_id(DEVICE_MAJOR_RAMDISK, 0),
			(UB*)MYNAME, port, sizeof(buf));
	if (result) {
		dbg_printf(MYNAME ": bind error=%d\n", result);
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
		dbg_printf(MYNAME ": start port=%d\n", port);

		while (accept(port) == E_OK);

		kcall->port_destroy(port);
		dbg_printf(MYNAME ": end\n");
	}

	kcall->thread_end_and_destroy();
}
