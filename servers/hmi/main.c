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
#include <services.h>
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "keyboard.h"
#include "mouse.h"

struct fs_func {
	void (*call)(fs_request *);
	size_t max;
};

ER_ID accept_tid = 0;
//static ER_ID hmi_tid = 0;
ID cons_mid;

static fs_request requests[REQUEST_QUEUE_SIZE];
volatile lfq_t req_queue;
static char req_buf[
	lfq_buf_size(sizeof(fs_request *), REQUEST_QUEUE_SIZE)
];
volatile lfq_t unused_queue;
static char unused_buf[
	lfq_buf_size(sizeof(fs_request *), REQUEST_QUEUE_SIZE)
];

Screen screen0;
esc_state_t state0;

static void no_support(fs_request *);
static ER accept(void);
static ER initialize(void);

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


static void no_support(fs_request *req)
{
	//TODO what is tag?
	reply_error(req, req->packet.header.token, 0, ENOTSUP);
}

static ER accept(void)
{
//TODO multiple interrupt is needed on mouse / keyboard?
	fs_request *req;
	while (lfq_dequeue(&unused_queue, &req) != QUEUE_OK)
		kcall->ipc_listen();

	ER_UINT size = kcall->ipc_receive(PORT_WINDOW, &(req->tag),
			&(req->packet));
	if (size < 0) {
		log_err("hmi: receive error=%d\n", size);
		return size;
	}

	if (size < sizeof(req->packet.header)) {
		//TODO what is tag?
		reply_error(req, 0, 0, EPROTO);
		return E_OK;
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

		func_table[req->packet.header.type].call(req);
		return E_OK;
	} while (false);

	//TODO what is tag?
	reply_error(req, req->packet.header.token, 0, result);
	return E_OK;
}

static ER initialize(void)
{
	state0.screen = &screen0;
	terminal_initialize(&state0);
	window_initialize();

	if (event_initialize())
		return E_SYS;

	lfq_initialize(&req_queue, req_buf, sizeof(fs_request *),
			REQUEST_QUEUE_SIZE);
	lfq_initialize(&unused_queue, unused_buf, sizeof(fs_request *),
			REQUEST_QUEUE_SIZE);

	for (int i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
		fs_request *p = &(requests[i]);
		lfq_enqueue(&unused_queue, &p);
	}

	T_CMTX pk_cmtx = {
		TA_TFIFO | TA_CEILING,
		pri_dispatcher
	};
	int result = kcall->mutex_create(accept_tid, &pk_cmtx);
	if (result) {
		log_err("hmi: mutex error=%d\n", result);
		return result;
	} else
		cons_mid = accept_tid;

	T_CPOR pk_cpor = {
		TA_TFIFO,
		sizeof(fsmsg_t),
		sizeof(fsmsg_t)
	};
	result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err("hmi: open error=%d\n", result);
		return result;
	}

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
	return E_OK;
}

void start(VP_INT exinf)
{
	accept_tid = kcall->thread_get_id();

	if (initialize() == E_OK) {
		log_info("hmi: start\n");

		if (keyboard_initialize() == E_OK)
			reader = get_char;

		mouse_initialize();

		while (accept() == E_OK);

//		kcall->thread_destroy(hmi_tid);
		kcall->ipc_close();
		log_info("hmi: end\n");
	}

	//TODO close port
	//TODO release mutex
	kcall->thread_end_and_destroy();
}
