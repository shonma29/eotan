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
#include <event.h>
#include <services.h>
#include <string.h>
#include <nerve/kcall.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <libserv.h>
#include "hmi.h"
#include "mouse.h"

static fs_request *current_req = NULL;
static volatile bool raw_mode;
volatile lfq_t hmi_queue;
static char int_buf[
	lfq_buf_size(sizeof(hmi_interrupt_t), INTERRUPT_QUEUE_SIZE)
];

static void process(const int);
static ER_UINT dummy_read(const int);

ER_UINT (*reader)(const int) = dummy_read;


void hmi_handle(const int type, const int data)
{
	for (;;) {
		hmi_interrupt_t in;
		if (lfq_dequeue(&hmi_queue, &in) == QUEUE_OK)
			switch (in.type) {
			case event_keyboard:
//				kcall->ipc_notify(hmi_tid, EVENT_IO);
				process(in.data);
				break;
			case event_mouse:
				mouse_process(in.type, in.data);
				break;
			default:
				break;
			}
		else
//			kcall->ipc_listen();
			break;
	}
}

static void process(const int data)
{
	ER_UINT d = reader(data);
	if (d < 0)
		return;

	if (!current_req) {
		if (lfq_dequeue(&req_queue, &current_req) == QUEUE_OK)
			current_req->packet.Tread.offset = 0;
		else
			return;
	}

	fsmsg_t *message = &(current_req->packet);
//	message->Tread.data[message->Tread.offset++] =
//			(unsigned char) (d & 0xff);
	unsigned char buf = (unsigned char) (d & 0xff);

	if (!raw_mode) {
		int result;
		// ^c
		if (buf == 0x03) {
			sys_args_t args = {
				syscall_kill,
				thread_id_of_token(message->header.token),
				SIGINT
			};

			result = kcall->ipc_send(PORT_MM, &args, sizeof(args));
			if (result)
				log_err("hmi: kill error=%d\n", result);

			goto terminate;
		}

		result = kcall->mutex_lock(cons_mid, TMO_FEVR);
		if (result)
			kcall->printk("hmi: handler cannot lock %d\n", result);
		else {
			mouse_hide();
			terminal_write((char *) &buf, &state0, 0, 1);
			mouse_show();
			result = kcall->mutex_unlock(cons_mid);
			if (result)
				kcall->printk("hmi: handler cannot unlock %d\n",
						result);
		}

		// ^d
		if (buf == 0x04)
			goto terminate;
	}

	//TODO loop or buffering (flush when user switches window, or LF)
	//TODO error check
	uintptr_t addr = ((uintptr_t) (message->Tread.data)
			+ message->Tread.offset);
	kcall->region_put(thread_id_of_token(message->header.token),
			(char *) addr, 1, &buf);
	message->Tread.offset++;
	if ((message->Tread.count <= message->Tread.offset)
			|| (buf == 0x0a)) {
terminate:
//		message->header.token = message->head.token;
		message->header.type = Rread;
//		message->Rread.tag = message->Tread.tag;
		message->Rread.count = message->Tread.offset;
		reply(current_req, MESSAGE_SIZE(Rread));
		current_req = NULL;
	}
}

static ER_UINT dummy_read(const int arg)
{
	return E_NOSPT;
}

ER_UINT consctl_write(const UW size, const char *inbuf)
{
	if (size == 5) {
		if (!memcmp(inbuf, "rawon", 5))
			raw_mode = true;
	} else if (size == 6) {
		if (!memcmp(inbuf, "rawoff", 6))
			raw_mode = false;
	}

	return size;
}

int event_initialize(void)
{
	lfq_initialize(&hmi_queue, int_buf, sizeof(hmi_interrupt_t),
			INTERRUPT_QUEUE_SIZE);
	return 0;
}
