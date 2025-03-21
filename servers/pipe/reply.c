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
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include "pipe.h"
#include "api.h"


int reply(const int tag, fsmsg_t *response, const size_t size)
{
	return (kcall->ipc_send(PORT_MNT, (void *) response, size) ?
			ECONNREFUSED : 0);
}

int reply_error(const int ipc_tag, const int token, const int caller_tag,
		const int error_no)
{
	fsmsg_t response;
	response.header.ident = IDENT;
	response.header.type = Rerror;
	response.header.token = (MYPORT << 16) | (token & 0xffff);
	response.Rerror.tag = caller_tag;
	response.Rerror.ename = error_no;
	return (kcall->ipc_send(PORT_MNT, &response, MESSAGE_SIZE(Rerror)) ?
			ECONNREFUSED : 0);
}

void reply_read(fs_request_t *req)
{
	fsmsg_t *response = &(req->packet);
	//response->header.token = req->packet.header.token;
	response->header.type = Rread;
	//response->Rread.tag = req->packet.Tread.tag;
	response->Rread.count = req->position;

	int result = reply(req->tag, response, MESSAGE_SIZE(Rread));
	if (result)
		log_warning(MYNAME ": failed to reply %d\n", result);
}

void reply_write(fs_request_t *req)
{
	fsmsg_t *response = &(req->packet);
	//response->header.token = req->packet.header.token;
	response->header.type = Rwrite;
	//response->Rwrite.tag = req->packet.Twrite.tag;
	response->Rwrite.count = req->position;

	int result = reply(req->tag, response, MESSAGE_SIZE(Rwrite));
	if (result)
		log_warning(MYNAME ": failed to reply %d\n", result);
}

void kill(const int thread_id, const int signal)
{
	sys_args_t args = {
		syscall_kill,
		thread_id,
		signal
	};
	int error_no = kcall->ipc_send(PORT_MM, &args, sizeof(args));
	if (error_no)
		log_err(MYNAME ": kill error %d\n", error_no);
}