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
#include <console.h>
#include <errno.h>
#include <event.h>
#include <services.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <dev/device.h>
#include <fs/protocol.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/ipc_utils.h>
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

extern void put(Screen *s, const unsigned int start, const size_t size,
		const char *buf);
extern void pset(Screen *s, unsigned int x, unsigned int y, int color);
#else
#include <cga.h>
#endif

static char line[4096];
Screen window[MAX_WINDOW];
static ER_ID receiver_tid = 0;
static ER_ID hmi_tid = 0;
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
		const char *inbuf);
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
				kcall->thread_wakeup(hmi_tid);
			else
				log_warning("hmi: int_queue is full\n");
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
			fsmsg_t *message;
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
//			message->Tread.data[message->Tread.offset++] =
//					(unsigned char)(d & 0xff);
			unsigned char buf[1];
			buf[0] = (unsigned char)(d & 0xff);

			//TODO loop
			//TODO error check
			unsigned int addr = ((unsigned int)(message->Tread.data)
					+ message->Tread.offset);
			kcall->region_put(port_of_ipc(message->header.token),
					(char*)addr, 1, buf);
			message->Tread.offset++;
			if (message->Tread.count <= message->Tread.offset) {
//				message->header.token = message->head.token;
				message->header.type = Rread;
//				message->Rread.tag = message->Tread.tag;
				message->Rread.count = message->Tread.count;
				reply(current_req, MESSAGE_SIZE(Rread));
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
		const char *inbuf)
{
	switch (dd) {
#ifdef USE_VESA
	case 4:
		put(&(window[0]), start, size, inbuf);
		break;

	case 5:
		if (size != sizeof(int) * 3)
			return E_PAR;
		else {
			unsigned int x = ((int*)inbuf)[0];
			unsigned int y = ((int*)inbuf)[1];
			int color = ((int*)inbuf)[2];
			pset(&(window[0]), x, y, color);
		}
		break;
	default:
		if (dd >= sizeof(window) / sizeof(window[0]))
			return E_PAR;

		{
			for (int i = 0; i < size; i++)
				cns->putc(&(window[dd]), inbuf[i]);
		}
#else
	default:
		{
			for (int i = 0; i < size; i++)
				cns->putc(&(window[0]), inbuf[i]);
		}
#endif
		break;
	}

	return size;
}

static void reply(request_message_t *req, const size_t size)
{
	ER_UINT result = kcall->ipc_send(req->tag, &(req->message), size);

	if (result)
		log_err("hmi: reply error=%d\n", result);

	lfq_enqueue(&unused_queue, &req);
	kcall->thread_wakeup(receiver_tid);
}

static void execute(request_message_t *req)
{
	fsmsg_t *message = &(req->message);
	ER_UINT result;
//TODO cancel request
	switch (message->header.type) {
	case Tread:
		result = check_param(message->Tread.offset,
				message->Tread.count);
		if (result) {
//			message->header.token = message->head.token;
			message->header.type = Rread;
//			message->Rread.tag = message->Tread.tag;
			message->Rread.count = result;
			reply(req, MESSAGE_SIZE(Rread));

		} else if (lfq_enqueue(&req_queue, &req) != QUEUE_OK) {
			log_debug("hmi: req_queue is full\n");
//			message->header.token = message->head.token;
			message->header.type = Rerror;
//			message->Rerror.tag = message->Tread.tag;
			message->Rerror.ename = ENOMEM;
			reply(req, MESSAGE_SIZE(Rerror));
		}
		break;

	case Twrite:
#ifdef USE_VESA
		if (message->Twrite.fid) {
			if (message->Twrite.count > sizeof(line))
				result = E_PAR;
			else if (kcall->region_get(port_of_ipc(req->tag),
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
#else
		{
#endif
			//TODO loop by bufsize
			unsigned int pos = 0;
			result = 0;
			for (size_t rest = message->Twrite.count; rest > 0;) {
				size_t len = (rest > sizeof(line)) ?
						sizeof(line) : rest;
				if (kcall->region_get(
						port_of_ipc(message->header.token),
						(char*)((unsigned int)(message->Twrite.data) + pos),
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
		kcall->thread_sleep();

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
	W result;
	W i;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(fsmsg_t),
			sizeof(fsmsg_t)
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
	Screen *s = &(window[0]);
	s->width /= 2;
	s->height /= 2;
	s->chr_width = s->width / s->font.width;
	s->chr_height = s->height / s->font.height;

	window[1] = window[0];
	s = &(window[1]);
	s->base += s->width * 3;
	s->p = (uint8_t*)(s->base);
	s->fgcolor.rgb.b = 31;
	s->fgcolor.rgb.g = 223;
	s->fgcolor.rgb.r = 0;
	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 31;
	s->bgcolor.rgb.r = 0;
	cns->cls(s);
	cns->locate(s, 0, 0);

	window[2] = window[0];
	s = &(window[2]);
	s->base += s->height * s->bpl;
	s->p = (uint8_t*)(s->base);
	s->fgcolor.rgb.b = 0;
	s->fgcolor.rgb.g = 127;
	s->fgcolor.rgb.r = 255;
	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 31;
	cns->cls(s);
	cns->locate(s, 0, 0);

	window[3] = window[0];
	s = &(window[3]);
	s->base += s->height * s->bpl + s->width * 3;
	s->p = (uint8_t*)(s->base);
	s->fgcolor.rgb.b = 0x30;
	s->fgcolor.rgb.g = 0x30;
	s->fgcolor.rgb.r = 0x30;
	s->bgcolor.rgb.b = 0xfc;
	s->bgcolor.rgb.g = 0xfc;
	s->bgcolor.rgb.r = 0xfc;
	cns->cls(s);
	cns->locate(s, 0, 0);
#else
	cns = getCgaConsole(&(window[0]),
			(const UH*)kern_p2v((void*)CGA_VRAM_ADDR));
#endif
	cns->cls(&(window[0]));
	cns->locate(&(window[0]), 0, 0);
//TODO create mutex
	result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err("hmi: open error=%d\n", result);

		return result;
	}

	result = kcall->thread_create_auto(&pk_ctsk);
	if (result < 0) {
		log_err("hmi: create error=%d\n", result);
		kcall->ipc_close();
		return result;
	}
	hmi_tid = result;

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

		kcall->thread_destroy(hmi_tid);
		kcall->ipc_close();
		log_info("hmi: end\n");
	}

	kcall->thread_end_and_destroy();
}
