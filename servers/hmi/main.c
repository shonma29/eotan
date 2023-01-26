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
#include <event.h>
#include <services.h>
#include <string.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "keyboard.h"
#include "mouse.h"

static char line[4096];
volatile bool raw_mode;
static ER_ID receiver_tid = 0;
//static ER_ID hmi_tid = 0;
ID cons_mid;
static request_message_t requests[REQUEST_QUEUE_SIZE];
volatile lfq_t req_queue;
static char req_buf[
	lfq_buf_size(sizeof(request_message_t *), REQUEST_QUEUE_SIZE)
];
static volatile lfq_t unused_queue;
static char unused_buf[
	lfq_buf_size(sizeof(request_message_t *), REQUEST_QUEUE_SIZE)
];
volatile lfq_t hmi_queue;

static char int_buf[
	lfq_buf_size(sizeof(hmi_interrupt_t), INTERRUPT_QUEUE_SIZE)
];

Screen screen0;
esc_state_t state0;

#ifdef USE_VESA
Display *display;
esc_state_t state2;
esc_state_t state7;
#endif

static ER_UINT write(const UW, const UW, const UW, const char *);
static void execute(request_message_t *);
static ER accept(void);
static ER initialize(void);
static ER_UINT dummy_read(const int);

ER_UINT (*reader)(const int) = dummy_read;


static ER_UINT write(const UW dd, const UW start, const UW size,
		const char *inbuf)
{
	switch (dd) {
	case 6:
		if (size == 5) {
			if (!memcmp(inbuf, "rawon", 5))
				raw_mode = true;
		} else if (size == 6) {
			if (!memcmp(inbuf, "rawoff", 6))
				raw_mode = false;
		}
		break;
#ifdef USE_VESA
	case 4:
		return draw_write(size, inbuf);
	default: {
		int result = kcall->mutex_lock(cons_mid, TMO_FEVR);
		if (result)
			kcall->printk("hmi: main cannot lock %d\n", result);
		else {
			mouse_hide();
			terminal_write((char *) inbuf,
					(dd ? ((dd == 7) ? &state7 : &state2) : &state0),
					0, size);
			mouse_show();
			result = kcall->mutex_unlock(cons_mid);
			if (result)
				kcall->printk("hmi: main cannot unlock %d\n",
						result);
		}
	}
#else
	default: {
		int result = kcall->mutex_lock(cons_mid, TMO_FEVR);
		if (result)
			kcall->printk("hmi: main cannot lock %d\n", result);
		else {
			terminal_write((char *) inbuf, &state0, 0, size);
			result = kcall->mutex_unlock(cons_mid);
			if (result)
				kcall->printk("hmi: main cannot unlock %d\n",
						result);
		}
	}
#endif
		break;
	}

	return size;
}

void reply(request_message_t *req, const size_t size)
{
	ER_UINT result = kcall->ipc_send(req->tag, &(req->message), size);
	if (result)
		log_err("hmi: reply error=%d\n", result);

	lfq_enqueue(&unused_queue, &req);
	kcall->ipc_notify(receiver_tid, EVENT_SERVICE);
}

static void execute(request_message_t *req)
{
	fsmsg_t *message = &(req->message);
	if (message->header.ident != IDENT) {
		message->header.ident = IDENT;
//		message->header.token = message->head.token;
		message->header.type = Rerror;
		message->Rerror.tag = 0;
		message->Rerror.ename = EPROTO;
		reply(req, MESSAGE_SIZE(Rerror));
		return;
	}

	ER_UINT result;
//TODO cancel request
	switch (message->header.type) {
	case Tread:
		if (lfq_enqueue(&req_queue, &req) != QUEUE_OK) {
			log_debug("hmi: req_queue is full\n");
//			message->header.token = message->head.token;
			message->header.type = Rerror;
//			message->Rerror.tag = message->Tread.tag;
			message->Rerror.ename = ENOMEM;
			reply(req, MESSAGE_SIZE(Rerror));
		}
		break;
	case Twrite:
		if (message->Twrite.fid) {
			if (message->Twrite.count > sizeof(line))
				result = E_PAR;
			else if (kcall->region_get(port_of_ipc(req->tag),
					message->Twrite.data,
					message->Twrite.count,
					line))
				result = E_SYS;
			else
				result = write(message->Twrite.fid,
						message->Twrite.offset,
						message->Twrite.count,
						line);
		} else {
			unsigned int pos = 0;
			result = 0;
			for (size_t rest = message->Twrite.count; rest > 0;) {
				size_t len = (rest > sizeof(line)) ?
						sizeof(line) : rest;
				if (kcall->region_get(
						(message->header.token >> 16) & 0xffff,
						(char *) ((unsigned int) (message->Twrite.data) + pos),
						len,
						line)) {
					result = E_SYS;
					break;
				} else {
					result = write(message->Twrite.fid,
							message->Twrite.offset,
							len,
							line);
					if (result < 0)
						break;
				}
				rest -= len;
				pos += len;
				message->Twrite.offset += len;
			}
//TODO return Rerror
			if (result >= 0)
				result = message->Twrite.count;
		}
		//TODO return Rerror if result is error
//		message->header.token = message->head.token;
		message->header.type = Rwrite;
//		message->Rwrite.tag = message->Twrite.tag;
		message->Rwrite.count = result;
		reply(req, MESSAGE_SIZE(Rwrite));
		break;
	case Tclunk:
//		message->header.token = message->head.token;
		message->header.type = Rclunk;
//		message->Rclunk.tag = message->Tclunk.tag;
		reply(req, MESSAGE_SIZE(Rclunk));
		break;
	default:
//		message->header.token = message->head.token;
		message->header.type = Rerror;
		message->Rerror.tag = 0;
		message->Rerror.ename = ENOTSUP;
		reply(req, MESSAGE_SIZE(Rerror));
		break;
	}
}

static ER accept(void)
{
	request_message_t *req;
	ER_UINT size;

//TODO multiple interrupt is needed on mouse / keyboard?
	while (lfq_dequeue(&unused_queue, &req) != QUEUE_OK)
		kcall->ipc_listen();

	size = kcall->ipc_receive(PORT_CONSOLE, &(req->tag), &(req->message));
	if (size < 0) {
		log_err("hmi: receive error=%d\n", size);
		return size;
	}

	execute(req);
	return E_OK;
}
static ER initialize(void)
{
#ifdef USE_VESA
	display = get_display();
	if (!display)
		return E_SYS;
#endif
	lfq_initialize(&hmi_queue, int_buf, sizeof(hmi_interrupt_t),
			INTERRUPT_QUEUE_SIZE);
	lfq_initialize(&req_queue, req_buf, sizeof(request_message_t *),
			REQUEST_QUEUE_SIZE);
	lfq_initialize(&unused_queue, unused_buf, sizeof(request_message_t *),
			REQUEST_QUEUE_SIZE);

	for (int i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
		request_message_t *p = &(requests[i]);
		lfq_enqueue(&unused_queue, &p);
	}

	state0.screen = &screen0;
	terminal_initialize(&state0);
#ifdef USE_VESA
	window_initialize();
#else
	if (sysinfo->cga)
		screen0 = *(sysinfo->cga);
	else
		terminal_write(STR_CONS_INIT, &state0, 0, LEN_CONS_INIT);
#endif
	T_CMTX pk_cmtx = {
		TA_TFIFO | TA_CEILING,
		pri_dispatcher
	};
	int result = kcall->mutex_create(receiver_tid, &pk_cmtx);
	if (result) {
		log_err("hmi: mutex error=%d\n", result);
		return result;
	} else
		cons_mid = receiver_tid;

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

static ER_UINT dummy_read(const int arg)
{
	return E_NOSPT;
}

void start(VP_INT exinf)
{
	receiver_tid = kcall->thread_get_id();

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
