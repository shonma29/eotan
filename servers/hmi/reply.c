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
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "api.h"
#include "session.h"

typedef struct {
	fsmsg_t *message;
	event_buf_t *ev;
	ssize_t offset;
	int thread_id;
} read_param_t;

static int _reply(fs_request_t *req, fsmsg_t *, const size_t);
static int _put(read_param_t *, int);


int reply(fs_request_t *req, const size_t size)
{
	return _reply(req, &(req->packet), size);
}

int reply_error(fs_request_t *req, const int token, const int caller_tag,
		const int error_no)
{
	fsmsg_t response;
	response.header.ident = IDENT;
	response.header.type = Rerror;
	response.header.token = (MYPORT << 16) | (token & 0xffff);
	response.Rerror.tag = caller_tag;
	response.Rerror.ename = error_no;
	return _reply(req, &response, MESSAGE_SIZE(Rerror));
}

static int _reply(fs_request_t *req, fsmsg_t *response, const size_t size)
{
	ER_UINT result = kcall->ipc_send(PORT_MNT, (void *) response, size);
	if (result)
		log_err(MYNAME ": reply error=%d\n", result);

	list_remove(&(req->brothers));
	list_remove(&(req->replies));
	slab_free(&request_slab, req);
	kcall->ipc_notify(MYPORT, EVENT_SERVICE);
	return (result ? ECONNREFUSED : 0);
}

int reply_read(fs_request_t *req)
{
	read_param_t par;
	par.message = &(req->packet);
	par.offset = 0;
	do {
		if (!(par.message->Tread.count))
			break;

		session_t *s = (session_t *) (req->session);
		par.ev = &(s->event);
		par.thread_id = thread_id_of_token(par.message->header.token);
		if (par.ev->write_position < par.ev->read_position) {
			int n = par.ev->size - par.ev->read_position;
			if (par.message->Tread.count < n)
				n = par.message->Tread.count;

			int error_no = _put(&par, n);
			if (error_no)
				return error_no;

			if (!(par.message->Tread.count))
				break;
		}

		if (par.ev->read_position < par.ev->write_position) {
			int n = par.ev->write_position - par.ev->read_position;
			if (par.message->Tread.count < n)
				n = par.message->Tread.count;

			int error_no = _put(&par, n);
			if (error_no)
				return error_no;
		}
	} while (false);

	par.message->header.type = Rread;
	par.message->Rread.count = par.offset;
	return reply(req, MESSAGE_SIZE(Rread));
}

static int _put(read_param_t *p, int len)
{
	if (kcall->region_put(p->thread_id,
			(char *) &(p->message->Tread.data[p->offset]),
			len, &(p->ev->buf[p->ev->read_position]))) {
		//TODO send SIGSEGV
		return EFAULT;
	}

	p->message->Tread.count -= len;
	p->ev->read_position += len;
	p->ev->read_position &= p->ev->position_mask;
	p->offset += len;
	return 0;
}
