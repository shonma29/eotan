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
#include <services.h>
#include <nerve/kcall.h>
#include <set/ring.h>
#include <libserv.h>
#include "syslog.h"

#define LEN_PRIORITY (2)

static char buf[SYSLOG_SIZE];

static ER check_param(const size_t);
static ssize_t read(char *, const size_t);
static ssize_t write(const int priority, char *, const size_t);
static size_t execute(syslog_t *);
static ER accept(const ID);
static ER_ID initialize(void);


static ER check_param(const size_t size)
{
	if (size > SYSLOG_MAX_LENGTH)
		return E_PAR;

	return E_OK;
}

static ssize_t read(char *outbuf, const size_t size)
{
	ER result = check_param(size);
	if (result)
		return result;


	return ring_get((ring_t *) buf, outbuf, size);
}

static ssize_t write(const int priority, char *inbuf, const size_t size)
{
	ER result = check_param(size);
	if (result)
		return result;

	if ((size + LEN_PRIORITY) > ring_get_rest((ring_t *) buf))
		return E_SYS;

	//TODO put current time
	//TODO put priority name
	char pri_msg[2] = {
		'0' + ((priority > LOG_DEBUG) ? 9 : priority), ' '
	};
	if (ring_put((ring_t *) buf, pri_msg, sizeof(pri_msg))
			!= sizeof(pri_msg))
		return E_SYS;

	result = ring_put((ring_t *) buf, inbuf, size);
	if (result != size)
		return E_SYS;

	return result;
}

static size_t execute(syslog_t *message)
{
	ssize_t result;
	size_t size = 0;

	switch (message->Tread.type) {
	case Tread:
		result = read(message->Rread.data, message->Tread.count);
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
		/*kcall->printk(MYNAME ": receive error=%d\n", size);*/
		return size;
	}

	int result = kcall->ipc_send(tag, &message, execute(&message));
	if (result) {
		/*kcall->printk(MYNAME ": reply error=%d\n", result);*/
	}

	return result;
}

static ER_ID initialize(void)
{
	ring_create(buf, sizeof(buf));

	T_CPOR pk_cpor = {
		TA_TFIFO,
		sizeof(syslog_t),
		sizeof(syslog_t)
	};
	ER err = kcall->ipc_open(&pk_cpor);
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

		while (accept(port) == E_OK);

		kcall->ipc_close();
		kcall->printk(MYNAME ": end\n");
	}

	kcall->thread_end_and_destroy();
}
