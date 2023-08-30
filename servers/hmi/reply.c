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
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <libserv.h>
#include "hmi.h"

static int _reply(fs_request *req, fsmsg_t *, const size_t);


int reply(fs_request *req, const size_t size)
{
	return _reply(req, &(req->packet), size);
}

int reply_error(fs_request *req, const int token, const int caller_tag,
		const int error_no)
{
	fsmsg_t response;
	response.header.ident = IDENT;
	response.header.type = Rerror;
	response.header.token = (PORT_WINDOW << 16) | (token & 0xffff);
	response.Rerror.tag = caller_tag;
	response.Rerror.ename = error_no;
	return _reply(req, &response, MESSAGE_SIZE(Rerror));
}

static int _reply(fs_request *req, fsmsg_t *response, const size_t size)
{
	ER_UINT result = kcall->ipc_send(req->tag, (void *) response, size);
	if (result)
		log_err("hmi: reply error=%d\n", result);

	lfq_enqueue(&unused_queue, &req);
	kcall->ipc_notify(accept_tid, EVENT_SERVICE);
	return (result ? ECONNREFUSED : 0);
}
