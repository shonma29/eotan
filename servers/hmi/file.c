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
#include <nerve/kcall.h>
#include <nerve/ipc_utils.h>
#include <libserv.h>
#include "hmi.h"
#include "mouse.h"

static char line[4096];

static ER_UINT write(const UW, const UW, const UW, const char *);


void if_clunk(fs_request *req)
{
	fsmsg_t *message = &(req->packet);

//	message->header.token = message->head.token;
	message->header.type = Rclunk;
//	message->Rclunk.tag = message->Tclunk.tag;
	reply(req, MESSAGE_SIZE(Rclunk));
}

void if_read(fs_request *req)
{
	if (lfq_enqueue(&req_queue, &req) != QUEUE_OK) {
		log_warning("hmi: req_queue is full\n");
		reply_error(req, req->packet.header.token,
				req->packet.Tread.tag, ENOMEM);
	}
}

void if_write(fs_request *req)
{
	ER_UINT result;
	fsmsg_t *message = &(req->packet);
	if ((message->Twrite.fid < 0)
			|| (message->Twrite.fid > 3)) {
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
//	message->header.token = message->head.token;
	message->header.type = Rwrite;
//	message->Rwrite.tag = message->Twrite.tag;
	message->Rwrite.count = result;
	reply(req, MESSAGE_SIZE(Rwrite));
}

static ER_UINT write(const UW dd, const UW start, const UW size,
		const char *inbuf)
{
	switch (dd) {
#ifdef USE_VESA
	case 4:
		return draw_write(size, inbuf);
#endif
	case 6:
		return consctl_write(size, inbuf);
	default: {
		int result = kcall->mutex_lock(cons_mid, TMO_FEVR);
		if (result)
			kcall->printk("hmi: main cannot lock %d\n", result);
		else {
#ifdef USE_VESA
			mouse_hide();
			terminal_write((char *) inbuf,
					(dd ? ((dd == 7) ? &state7 : &state2) : &state0),
					0, size);
			mouse_show();
#else
			terminal_write((char *) inbuf, &state0, 0, size);
#endif
			result = kcall->mutex_unlock(cons_mid);
			if (result)
				kcall->printk("hmi: main cannot unlock %d\n",
						result);
		}
	}
		break;
	}

	return size;
}
