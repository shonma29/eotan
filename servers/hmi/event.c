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
#include <string.h>
#include <event.h>
#include <nerve/kcall.h>
#include <win/keyboard.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <libserv.h>
#include "hmi.h"
#include "session.h"
#include "keyboard.h"
#include "mouse.h"

#define MASK_EVENT_TYPE (1)

#define SIZE_OF_EVENT (sizeof(event_message_t))

static bool raw_mode;

volatile lfq_t interrupt_queue;
static char interrupt_buf[
	lfq_buf_size(sizeof(hmi_interrupt_t), INTERRUPT_QUEUE_SIZE)
];

static void _process_keyboard(const int);
static void _enqueue_to_cons(const int);
static bool _event_is_full(event_buf_t *);
static void _event_putc(event_buf_t *, const char);
static void _enqueue_to_event(const event_message_t *);
static bool _event_is_full_for_message(event_buf_t *);
static void _event_put(event_buf_t *, const event_message_t *);

static void (*processors[])(const int) = {
	_process_keyboard,
	window_focus
};


void event_process_interrupt(void)
{
	for (hmi_interrupt_t event; !lfq_dequeue(&interrupt_queue, &event);) {
		if ((event.type & (~MASK_EVENT_TYPE)))
			continue;

		processors[event.type & MASK_EVENT_TYPE](event.data);
	}

	//TODO reply altogether here
}

static void _process_keyboard(const int data)
{
	int ascii_code = keyboard_convert(data);
	if (ascii_code < 0)
		return;

	if (!focused_session)
		return;

	switch (focused_session->type) {
	case TYPE_CONS:
		_enqueue_to_cons(ascii_code);
		break;
	case TYPE_WINDOW: {
			event_message_t message = { event_keyboard, data };
			_enqueue_to_event(&message);
		}
		break;
	default:
		break;
	}
}

static void _enqueue_to_cons(const int ascii_code)
{
	unsigned char buf = (unsigned char) (ascii_code & 0xff);

	if (!raw_mode) {
		if (buf == 0x03) {
			// ^c
			sys_args_t args = {
				syscall_kill,
				focused_session->tid,
				SIGINT
			};
			int error_no = kcall->ipc_send(PORT_MM, &args,
					sizeof(args));
			if (error_no)
				log_err(MYNAME ": kill error %d\n",
						error_no);

			return;
		} else if (buf == 0x04) {
			// ^d
			event_buf_t *p = &(focused_session->event);
			list_t *head = list_dequeue(&(p->readers));
			if (head) {
				fs_request_t *req = getRequestFromQueue(head);
				int error_no = reply_read(req);
				if (error_no)
					log_warning(MYNAME ": reply error %d\n",
							error_no);
			}

			return;
		}

		mouse_hide();
		terminal_write((char *) &buf, focused_session->state, 1);
		mouse_show();
	}

	event_buf_t *p = &(focused_session->event);
	if (list_is_empty(&(p->readers))) {
		if (_event_is_full(p)) {
			log_warning(MYNAME ": event overflow %d\n",
					focused_session->node.key);
			//TODO notify 'overflow'
			p->write_position = p->read_position;
		}

		_event_putc(p, buf);
	} else {
		//TODO !enqueue to reply list if less than size
		_event_putc(p, buf);

		list_t *head = list_dequeue(&(p->readers));
		fs_request_t *req = getRequestFromQueue(head);
		int error_no = reply_read(req);
		if (error_no)
			log_warning(MYNAME ": reply error %d\n", error_no);
	}
}

static bool _event_is_full(event_buf_t *p)
{
	if (p->read_position)
		return (p->write_position == (p->read_position - 1));
	else
		return (p->write_position == (p->size - 1));
}

static void _event_putc(event_buf_t *p, const char c)
{
	p->buf[p->write_position] = c;
	p->write_position++;
	p->write_position &= p->position_mask;
}

static void _enqueue_to_event(const event_message_t *message)
{
	event_buf_t *p = &(focused_session->event);
	if (list_is_empty(&(p->readers))) {
		if (_event_is_full_for_message(p)) {
			log_warning(MYNAME ": event overflow %d\n",
					focused_session->node.key);
			//TODO notify 'overflow'
			p->write_position = p->read_position;
		}

		_event_put(p, message);
	} else {
		//TODO !enqueue to reply list if less than size
		_event_put(p, message);

		list_t *head = list_dequeue(&(p->readers));
		fs_request_t *req = getRequestFromQueue(head);
		int error_no = reply_read(req);
		if (error_no)
			log_warning(MYNAME ": reply error %d\n", error_no);
	}
}

static bool _event_is_full_for_message(event_buf_t *p)
{
	if (p->read_position)
		return (p->write_position == (p->read_position - SIZE_OF_EVENT));
	else
		return (p->write_position == (p->size - SIZE_OF_EVENT));
}

static void _event_put(event_buf_t *p, const event_message_t *message)
{
	event_message_t *q = (event_message_t *) &(p->buf[p->write_position]);
	*q = *message;
	p->write_position += SIZE_OF_EVENT;
	p->write_position &= p->position_mask;
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
	lfq_initialize(&interrupt_queue, interrupt_buf, sizeof(hmi_interrupt_t),
			INTERRUPT_QUEUE_SIZE);
	return 0;
}
