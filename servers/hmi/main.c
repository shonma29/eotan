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
	REQUEST_SLAB = 0x08,
	PORT = 0x10
};

struct fs_func {
	int (*call)(fs_request_t *);
	size_t max;
};

Display *display = &(sysinfo->display);

slab_t request_slab;
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
	fs_request_t *req = slab_alloc(&request_slab);
	if (!req) {
		kcall->ipc_listen();
		event_process_interrupt();
		return 0;
	}

	list_initialize(&(req->queue));
	list_initialize(&(req->brothers));
	list_initialize(&(req->replies));
	req->session = NULL;

	int result;
	for (;;) {
		ER_UINT size = kcall->ipc_receive(PORT_WINDOW, &(req->tag),
				&(req->packet));
		if (size == E_RLWAI) {
			event_process_interrupt();
			continue;
		}

		if (size < 0) {
			log_err(MYNAME ": receive error %d\n", size);
			slab_free(&request_slab, req);
			return size;
		}

		if (size < sizeof(req->packet.header)) {
			reply_error(req, 0, 0, EPROTO);
			return 0;
		}

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
	}

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

	request_slab.unit_size = sizeof(fs_request_t);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(REQUEST_QUEUE_SIZE, PAGE_SIZE,
		sizeof(fs_request_t));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;

	if (slab_create(&request_slab))
		return REQUEST_SLAB;
	else
		initialized_resources |= REQUEST_SLAB;

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

	keyboard_initialize();
	mouse_initialize();
	return 0;
}

void start(VP_INT exinf)
{
	int result = initialize();
	if (result)
		log_err(MYNAME ": failed to initialize %d\n", result);
	else {
		log_info(MYNAME ": start\n");

		while (!accept());

		log_info(MYNAME ": end\n");
		result = kcall->ipc_close();
		if (result)
			log_err(MYNAME ": failed to close %d\n", result);

	}

	kcall->thread_end_and_destroy();
}
