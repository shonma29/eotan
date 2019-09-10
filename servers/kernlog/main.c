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
#include <dev/device.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <set/ring.h>
#include <sync.h>
#include <libserv.h>
#include "kernlog.h"

#define LEN_PRIORITY (2)

static char buf[SYSLOG_SIZE];

static ER check_param(const size_t);
static size_t lfcopy(char *, volatile lfq_t*, const size_t);
static size_t rcopy(char *, ring_t *, const size_t);
static ssize_t read(char *, const int, const size_t);
static ssize_t write(const int priority, char *, const size_t);
static size_t execute(syslog_t *);
static ER accept(const ID);
static ER_ID initialize(void);

#if USE_MONITOR
static ER monitor_initialize(void);
static void monitor(void);
static int write_cons(char *, const int, const off_t, const size_t);
static unsigned int sleep(unsigned int);
#endif

static ER check_param(const size_t size)
{
	if (size > SYSLOG_MAX_LENGTH)
		return E_PAR;

	return E_OK;
}

static size_t lfcopy(char *outbuf, volatile lfq_t* q, const size_t size)
{
	size_t left;

	for (left = size; left > 0; left--) {
		int w;

		if (lfq_dequeue(q, &w) != QUEUE_OK)
			break;

		*outbuf = (UB)(w & 0xff);
		outbuf++;
	}

	return size - left;
}

static size_t rcopy(char *outbuf, ring_t *r, const size_t size)
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

static ssize_t read(char *outbuf, const int channel, const size_t size)
{
	ER result = check_param(size);
	if (result)
		return result;

	switch (channel) {
	case channel_kernlog:
		return lfcopy(outbuf, (volatile lfq_t*)KERNEL_LOG_ADDR, size);

	case channel_syslog:
		return rcopy(outbuf, (ring_t*)buf, size);

	default:
		return E_PAR;
	}
}

static ssize_t write(const int priority, char *inbuf,
		const size_t size)
{
	ER result = check_param(size);
	if (result)
		return result;

	if (size + LEN_PRIORITY > RING_MAX_LEN)
		return E_PAR;

	//TODO put current time
	//TODO put priority name
	char pri_msg[2] = {
		'0' + ((priority > LOG_DEBUG) ? 9 : priority), ' '
	};
	if (ring_put((ring_t*)buf, pri_msg, sizeof(pri_msg)) < 0)
		return E_SYS;

	result = ring_put((ring_t*)buf, inbuf, size);
	if (result < 0)
		return E_SYS;

	return result;
}

static size_t execute(syslog_t *message)
{
	ssize_t result;
	size_t size = 0;

	switch (message->Tread.type) {
	case Tread:
		result = read(message->Rread.data,
				message->Tread.fid,
				message->Tread.count);
		message->Rread.count = result;
		size = sizeof(message->Rread)
				- sizeof(message->Rread.data)
				+ ((result > 0) ? message->Rread.count : 0);
		break;

	case Twrite:
		result = write(message->Twrite.priority, message->Twrite.data,
				message->Twrite.count);
		message->Rwrite.count = result;
		size = sizeof(message->Rwrite);
		break;

	default:
		break;
	}

	return size;
}

static ER accept(const ID port)
{
	syslog_t message;
	int tag;
	int size = kcall->ipc_receive(port, &tag, &message);
	if (size < 0) {
		/*log_err(MYNAME ": receive error=%d\n", size);*/
		return size;
	}

	int result = kcall->ipc_reply(tag, &message, execute(&message));
	if (result) {
		/*log_err(MYNAME ": reply error=%d\n", result);*/
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

	ring_create(buf, sizeof(buf));

	err = kcall->ipc_open(&pk_cpor);
	if (err) {
		/*log_err(MYNAME ": open error=%d\n", err);*/

		return err;
	}

	return PORT_SYSLOG;
}

void start(VP_INT exinf)
{
	ER_ID port = initialize();

	if (port >= 0) {
		kcall->printk(MYNAME ": start port=%d\n", port);
#if USE_MONITOR
		monitor_initialize();
#endif
		while (accept(port) == E_OK);

		kcall->ipc_close();
		kcall->printk(MYNAME ": end\n");
	}

	kcall->thread_end_and_destroy();
}

#if USE_MONITOR
static ER monitor_initialize(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, monitor, pri_server_middle,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	return kcall->thread_create_auto(&pk_ctsk);
}

static void monitor(void)
{
	kcall->printk("monitor: start\n");

	for (;;) {
		sleep(1);

		char outbuf[1024];

		for (size_t len;
				(len  = lfcopy(outbuf,
						(volatile lfq_t*)KERNEL_LOG_ADDR,
						sizeof(outbuf)));)
			write_cons(outbuf, 1, 0, len);

		for (size_t len;
				(len = rcopy(outbuf, (ring_t*)buf,
						sizeof(outbuf)));)
			write_cons(outbuf, 3, 0, len);
	}
}

//TODO use cons driver
static int write_cons(char *inbuf, const int channel,
		const off_t start, const size_t size)
{
	off_t rpos = 0;
	off_t wpos = start;
	size_t rest = size;

	while (rest > 0) {
		ER_UINT result;
		size_t len = (rest < DEV_BUF_SIZE) ? rest : DEV_BUF_SIZE;
		fsmsg_t packet;

		packet.header.type = Twrite;
		packet.Twrite.fid = channel;
		packet.Twrite.offset = wpos;
		packet.Twrite.count = len;
		packet.Twrite.data = &(inbuf[rpos]);

		result = kcall->ipc_call(PORT_CONSOLE, &packet,
				MESSAGE_SIZE(Twrite));
		if (result != MESSAGE_SIZE(Rwrite)) {
			kcall->printk("cons: call failed(%d)\n", result);
			return -1;
		}

		else if (packet.Rwrite.count != len) {
			kcall->printk("cons: wrote icompletely\n");
			return -1;
		}

		rpos += len;
		wpos += len;
		rest -= len;
	}

	return size;
}

//TODO extract to libserv
static unsigned int sleep(unsigned int second)
{
	struct timespec t = { second, 0 };
	ER_UINT reply_size = kcall->ipc_call(PORT_TIMER, &t, sizeof(t));

	if (reply_size == sizeof(ER)) {
		ER *result = (ER*)&t;

		switch (*result) {
		case E_TMOUT:
			return 0;

		case E_PAR:
			return second;

		case E_NOMEM:
			return second;

		default:
			break;
		}
	}

	return second;
}
#endif
