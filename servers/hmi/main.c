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
#include <core/options.h>
#include <console.h>
#include <device.h>
#include <event.h>
#include <services.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <set/list.h>
#include <libserv.h>
#include "hmi.h"
#include "keyboard.h"
#include "mouse.h"

#ifdef USE_VESA
#include <vesa.h>
#include "font.h"

static unsigned char line[4096];

extern void put(const unsigned int start, const size_t size,
		const unsigned char *buf);
extern void pset(unsigned int x, unsigned int y, int color);
#else
#include <cga.h>
#endif

static Screen window[MAX_WINDOW];
static ER_ID receiver_tid = 0;
static request_message_t *current_req = NULL;
static request_message_t requests[REQUEST_QUEUE_SIZE];
static volatile lfq_t req_queue;
static char req_buf[
		lfq_buf_size(sizeof(request_message_t*), REQUEST_QUEUE_SIZE)
];
static volatile lfq_t unused_queue;
static char unused_buf[
		lfq_buf_size(sizeof(request_message_t*), REQUEST_QUEUE_SIZE)
];
static volatile lfq_t int_queue;

static char int_buf[
		lfq_buf_size(sizeof(interrupt_message_t), INTERRUPT_QUEUE_SIZE)
];
static Console *cns;

static void process(const int arg);
static ER check_param(const UW start, const UW size);
static ER_UINT write(const UW dd, const UW start, const UW size,
		const UB *inbuf);
static void reply(request_message_t *req, const size_t size);
static void execute(request_message_t *req);
static ER accept(void);
static ER initialize(void);
static ER_UINT dummy_read(const int);

static ER_UINT (*reader)(const int) = dummy_read;


void hmi_handle(const int type, const int data)
{
	switch (type) {
	case event_keyboard:
		{
			interrupt_message_t message = { type, data };

			if (lfq_enqueue(&int_queue, &message) == QUEUE_OK)
				kcall->thread_wakeup(PORT_HMI);
			else
				dbg_printf("hmi: int_queue is full\n");
			break;
		}
	case event_mouse:
		mouse_process(type, data);
		break;

	default:
		break;
	}
}

static void process(const int arg)
{
	for (;;) {
		interrupt_message_t data;

		if (lfq_dequeue(&int_queue, &data) == QUEUE_OK) {
			devmsg_t *message;
			VP_INT d;

			switch (data.type) {
			case event_keyboard:
				d = reader(data.data);
				if (d < 0)
					continue;
				break;
			default:
				continue;
			}

			if (!current_req) {
				if (lfq_dequeue(&req_queue, &current_req)
						== QUEUE_OK)
					current_req->message.Tread.offset = 0;
				else
					continue;
			}

			message = &(current_req->message);
			message->Tread.data[message->Tread.offset++] =
					(unsigned char)(d & 0xff);
			if (message->Tread.count <= message->Tread.offset) {
				message->Rread.count = message->Tread.count;
				reply(current_req, sizeof(message->Rread));
				current_req = NULL;
			}
		} else
			kcall->thread_sleep();
	}
}

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

	if (result)
		return result;

	switch (dd) {
#ifdef USE_VESA
	case 1:
		put(start, size, inbuf);
		break;

	case 2:
		if (size != sizeof(int) * 3)
			return E_PAR;
		else {
			unsigned int x = ((int*)inbuf)[0];
			unsigned int y = ((int*)inbuf)[1];
			int color = ((int*)inbuf)[2];
			pset(x, y, color);
		}
		break;
#endif
	default:
		{
			size_t i;

			for (i = 0; i < size; i++)
				cns->putc(&(window[0]), inbuf[i]);
		}
		break;
	}

	return size;
}

static void reply(request_message_t *req, const size_t size)
{
	ER_UINT result = kcall->port_reply(req->rdvno, &(req->message), size);

	if (result)
		dbg_printf("hmi: rpl_rdv error=%d\n", result);

	lfq_enqueue(&unused_queue, &req);
	kcall->thread_wakeup(receiver_tid);
}

static void execute(request_message_t *req)
{
	devmsg_t *message = &(req->message);
	ER_UINT result;
//TODO cancel request
	switch (message->Tread.operation) {
	case operation_read:
		result = check_param(message->Tread.offset,
				message->Tread.count);
		if (result) {
			message->Rread.count = result;
			reply(req, sizeof(message->Rread));

		} else if (lfq_enqueue(&req_queue, &req) != QUEUE_OK) {
			dbg_printf("hmi: req_queue is full\n");
			message->Rread.count = E_NOMEM;
			reply(req, sizeof(message->Rread));
		}
		break;

	case operation_write:
#ifdef USE_VESA
		if (message->Twrite.fid) {
			if (kcall->region_get(get_rdv_tid(req->rdvno),
					message->Twrite.data,
					message->Twrite.count,
					line)) {
				result = E_SYS;
			} else {
				result = write(message->Twrite.fid,
						message->Twrite.offset,
						message->Twrite.count,
						line);
			}
		} else {
#endif
			result = write(message->Twrite.fid,
					message->Twrite.offset,
					message->Twrite.count,
					message->Twrite.data);
#ifdef USE_VESA
		}
#endif
		message->Rwrite.count = result;
		reply(req, sizeof(message->Rwrite));
		break;

	default:
		break;
	}
}

static ER accept(void)
{
	request_message_t *req;
	ER_UINT size;

//TODO multiple interrupt is needed on mouse / keyboard?
	while (lfq_dequeue(&unused_queue, &req) != QUEUE_OK)
		kcall->thread_sleep();

	size = kcall->port_accept(PORT_CONSOLE, &(req->rdvno), &(req->message));
	if (size < 0) {
		dbg_printf("hmi: acp_por error=%d\n", size);
		return size;
	}

	execute(req);
	return E_OK;
}

static ER initialize(void)
{
	W result;
	W i;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(devmsg_t),
			sizeof(devmsg_t)
	};
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, process, pri_server_middle,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	lfq_initialize(&int_queue, int_buf, sizeof(interrupt_message_t),
				INTERRUPT_QUEUE_SIZE);
	lfq_initialize(&req_queue, req_buf, sizeof(request_message_t*),
				REQUEST_QUEUE_SIZE);
	lfq_initialize(&unused_queue, unused_buf, sizeof(request_message_t*),
				REQUEST_QUEUE_SIZE);
	for (i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
		request_message_t *p = &(requests[i]);

		lfq_enqueue(&unused_queue, &p);
	}
#ifdef USE_VESA
	cns = getVesaConsole(&(window[0]), &default_font);
#else
	cns = getCgaConsole(&(window[0]),
			(const UH*)kern_p2v((void*)CGA_VRAM_ADDR));
#endif
	cns->cls(&(window[0]));
	cns->locate(&(window[0]), 0, 0);
//TODO create mutex
	result = kcall->port_open(&pk_cpor);
	if (result) {
		dbg_printf("hmi: cre_por error=%d\n", result);

		return result;
	}

	result = kcall->thread_create(PORT_HMI, &pk_ctsk);
	if (result) {
		dbg_printf("hmi: cre_tsk failed %d\n", result);
		kcall->port_close();
		return result;
	}

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
		dbg_printf("hmi: start\n");

		if (keyboard_initialize() == E_OK)
			reader = get_char;

		mouse_initialize();

		while (accept() == E_OK);

		kcall->thread_destroy(PORT_HMI);
		kcall->port_close();
		dbg_printf("hmi: end\n");
	}

	kcall->thread_end_and_destroy();
}
