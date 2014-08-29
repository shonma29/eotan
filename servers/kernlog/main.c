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
#include <services.h>
#include <string.h>
#include <syslog.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <set/ring.h>
#include <sync.h>
#include <libserv.h>
#include "kernlog.h"

static unsigned char buf[SYSLOG_SIZE];

static ER check_param(const size_t);
static size_t lfcopy(unsigned char *, volatile lfq_t*, const size_t);
static size_t rcopy(unsigned char *, ring_t *, const size_t);
static ssize_t read(unsigned char *, const int, const size_t);
static ssize_t write(unsigned char *, const size_t);
static size_t execute(syslog_t *);
static ER accept(const ID);
static ER_ID initialize(void);


static ER check_param(const size_t size)
{
	if (size > SYSLOG_MAX_LENGTH)	return E_PAR;

	return E_OK;
}

static size_t lfcopy(unsigned char *outbuf, volatile lfq_t* q, const size_t size)
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

static size_t rcopy(unsigned char *outbuf, ring_t *r, const size_t size)
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

static ssize_t read(unsigned char *outbuf, const int channel, const size_t size)
{
	ER result = check_param(size);

	if (result)	return result;
	if (size != RING_MAX_LEN)	return E_PAR;

	switch (channel) {
	case channel_kernlog:
		return lfcopy(outbuf, (volatile lfq_t*)KERNEL_LOG_ADDR, size);

	case channel_syslog:
		return rcopy(outbuf, (ring_t*)buf, size);

	default:
		return E_PAR;
	}
}

static ssize_t write(unsigned char *inbuf, const size_t size)
{
	ER result = check_param(size);

	if (result)	return result;
	if (size > RING_MAX_LEN)	return E_PAR;

	result = ring_put((ring_t*)buf, inbuf, size);

	if (result < 0)
		return E_SYS;

	return result;
}

static size_t execute(syslog_t *message)
{
	ssize_t result;
	size_t size = 0;

	switch (message->Rread.operation) {
	case operation_read:
		result = read(message->Tread.data,
				message->Rread.channel,
				message->Rread.length);
		message->Tread.length = result;
		size = sizeof(message->Tread)
				- sizeof(message->Tread.data)
				+ (message->Tread.length);
		break;

	case operation_write:
		result = write(message->Rwrite.data,
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
	syslog_t message;
	RDVNO rdvno;
	ER_UINT size;
	ER result;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	size = kcall->port_accept(port, &rdvno, &message);
	if (size < 0) {
		/*dbg_printf("[KERNLOG] acp_por error=%d\n", size);*/
		return size;
	}

	result = kcall->port_reply(rdvno, &message, execute(&message));
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
			sizeof(syslog_t),
			sizeof(syslog_t)
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
