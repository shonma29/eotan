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
#include <errno.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "session.h"
#include "keyboard.h"
#include "mouse.h"

enum {
	SESSIONS = 0x01,
	WINDOWS = 0x02,
	EVENTS = 0x04,
	REQ_QUEUE = 0x08,
	UNUSED_QUEUE = 0x10,
	MUTEX = 0x20,
	PORT = 0x40
};

struct fs_func {
	int (*call)(fs_request_t *);
	size_t max;
};

Display *display = &(sysinfo->display);
ER_ID accept_tid = 0;
//static ER_ID hmi_tid = 0;
ID cons_mid;

static fs_request_t requests[REQUEST_QUEUE_SIZE];
volatile lfq_t req_queue;
static char req_buf[
	lfq_buf_size(sizeof(fs_request_t *), REQUEST_QUEUE_SIZE)
];
volatile lfq_t unused_queue;
static char unused_buf[
	lfq_buf_size(sizeof(fs_request_t *), REQUEST_QUEUE_SIZE)
];
static int initialized_resources = 0;

static int no_support(fs_request_t *);
static int accept(void);
static int initialize(void);

//TODO cancel request?
static struct fs_func func_table[] = {
	{ if_attach, MESSAGE_SIZE(Tattach) },
	{ if_walk, MESSAGE_SIZE(Twalk) },
	{ if_open, MESSAGE_SIZE(Topen) },
	{ no_support, MESSAGE_SIZE(Tcreate) },
	{ if_read, MESSAGE_SIZE(Tread) },
	{ if_write, MESSAGE_SIZE(Twrite) },
	{ if_clunk, MESSAGE_SIZE(Tclunk) },
	{ no_support, MESSAGE_SIZE(Tremove) },
	{ no_support, MESSAGE_SIZE(Tstat) },
	{ no_support, MESSAGE_SIZE(Twstat) }
};
#define NUM_OF_FUNC (sizeof(func_table) / sizeof(func_table[0]))


static int no_support(fs_request_t *req)
{
	//TODO what is tag?
	reply_error(req, req->packet.header.token, 0, ENOTSUP);
	return 0;
}

static int accept(void)
{
	fs_request_t *req;
	while (lfq_dequeue(&unused_queue, &req) != QUEUE_OK)
		kcall->ipc_listen();

	req->session = NULL;

	ER_UINT size = kcall->ipc_receive(PORT_WINDOW, &(req->tag),
			&(req->packet));
	if (size < 0) {
		log_err(MYNAME ": receive error=%d\n", size);
		return size;
	}

	if (size < sizeof(req->packet.header)) {
		reply_error(req, 0, 0, EPROTO);
		return 0;
	}

	int result;
	do {
		if (req->packet.header.ident != IDENT) {
			result = EPROTO;
			break;
		}

		if (req->packet.header.type >= NUM_OF_FUNC) {
			result = ENOTSUP;
			break;
		}

		if (size != func_table[req->packet.header.type].max) {
			result = EPROTO;
			break;
		}

		int result = func_table[req->packet.header.type].call(req);
		if (result)
			reply_error(req, req->packet.header.token,
					req->packet.Rerror.tag, result);

		return 0;
	} while (false);

	reply_error(req, req->packet.header.token, 0, result);
	return 0;
}

static int initialize(void)
{
	int result = session_initialize();
	if (result) {
		log_err(MYNAME ": session error %d\n", result);
		return SESSIONS;
	} else
		initialized_resources |= SESSIONS;

	if (window_initialize())
		return WINDOWS;
	else
		initialized_resources |= WINDOWS;

	if (event_initialize())
		return EVENTS;
	else
		initialized_resources |= EVENTS;

	if (lfq_initialize(&req_queue, req_buf, sizeof(fs_request_t *),
			REQUEST_QUEUE_SIZE))
		return REQ_QUEUE;
	else
		initialized_resources |= REQ_QUEUE;

	if (lfq_initialize(&unused_queue, unused_buf, sizeof(fs_request_t *),
			REQUEST_QUEUE_SIZE))
		return UNUSED_QUEUE;
	else
		initialized_resources |= UNUSED_QUEUE;

	for (int i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
		fs_request_t *p = &(requests[i]);
		lfq_enqueue(&unused_queue, &p);
	}

	T_CMTX pk_cmtx = {
		TA_TFIFO | TA_CEILING,
		pri_dispatcher
	};
	result = kcall->mutex_create(accept_tid, &pk_cmtx);
	if (result) {
		log_err(MYNAME ": mutex error %d\n", result);
		return MUTEX;
	} else {
		cons_mid = accept_tid;
		initialized_resources |= MUTEX;
	}

	T_CPOR pk_cpor = {
		TA_TFIFO,
		sizeof(fsmsg_t),
		sizeof(fsmsg_t)
	};
	result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": failed to open %d\n", result);
		return PORT;
	} else
		initialized_resources |= PORT;

//	T_CTSK pk_ctsk = {
//		TA_HLNG | TA_ACT, 0, process, pri_server_middle,
//		KTHREAD_STACK_SIZE, NULL, NULL, NULL
//	};
//	result = kcall->thread_create_auto(&pk_ctsk);
//	if (result < 0) {
//		log_err("hmi: create error=%d\n", result);
//		kcall->ipc_close();
//		return result;
//	}

//	hmi_tid = result;
	return 0;
}

void start(VP_INT exinf)
{
	accept_tid = kcall->thread_get_id();

	int result = initialize();
	if (result)
		log_err(MYNAME ": failed to initialize %d\n", result);
	else {
		log_info(MYNAME ": start\n");

		if (!keyboard_initialize())
			reader = get_char;

		mouse_initialize();

		while (!accept());

		log_info(MYNAME ": end\n");
//		kcall->thread_destroy(hmi_tid);
		result = kcall->ipc_close();
		if (result)
			log_err(MYNAME ": failed to close %d\n", result);

	}

	//TODO close port
	//TODO release mutex
	kcall->thread_end_and_destroy();
}
