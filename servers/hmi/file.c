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
#include <fcntl.h>
#include <fs/vfs.h>
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "api.h"
#include "session.h"
#include "mouse.h"

#define MASK_UNSUPPORTED_MODE (O_APPEND | O_TRUNC)

static char line[4096];

static ER_UINT _write(struct file *, const UW, const UW, const char *);


int if_open(fs_request_t *req)
{
	int error_no;
	struct _Topen *request = &(req->packet.Topen);
	do {
		session_t *session = session_find_by_request(req);
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		//TODO support access to directory
		if (!(file->f_channel)) {
			error_no = EISDIR;
			break;
		}

		if (file->f_flag != O_ACCMODE) {
			error_no = EBUSY;
			break;
		}

		if (request->mode & O_EXEC) {
			error_no = EACCES;
			break;
		}

		if (!vfs_check_flags(request->mode)
				|| (request->mode & MASK_UNSUPPORTED_MODE)) {
			error_no = EINVAL;
			break;
		}

		file->f_flag = request->mode & O_ACCMODE;

		fsmsg_t *response = &(req->packet);
		//response->header.token = req->packet.header.token;
		response->header.type = Ropen;
		//response->Ropen.tag = request->tag;
		response->Ropen.qid = file->node.key;
		response->Ropen.iounit = 0;
		reply(req, MESSAGE_SIZE(Ropen));
		return 0;
	} while (false);

	return error_no;
}

int if_clunk(fs_request_t *req)
{
	int error_no;
	struct _Tclunk *request = &(req->packet.Tclunk);
	do {
		session_t *session = session_find_by_request(req);
		if (!session) {
			error_no = EPERM;
			break;
		}

		error_no = session_destroy_file(session, request->fid);
		if (error_no)
			break;

		fsmsg_t *response = &(req->packet);
		//response->header.token = req->packet.header.token;
		response->header.type = Rclunk;
		//response->Rclunk.tag = request->tag;
		reply(req, MESSAGE_SIZE(Rclunk));
		return 0;
	} while (false);

	return error_no;
}

int if_read(fs_request_t *req)
{
	int error_no;
	struct _Tread *request = &(req->packet.Tread);
	do {
		session_t *session = session_find_by_request(req);
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
		if (!file
				|| (file->f_flag == O_WRONLY)
				|| (file->f_flag == O_ACCMODE)) {
			error_no = EBADF;
			break;
		}

		if (lfq_enqueue(&req_queue, &req) != QUEUE_OK) {
			log_warning(MYNAME ": req_queue is full\n");
			reply_error(req, req->packet.header.token,
					req->packet.Tread.tag, ENOMEM);
		}

		return 0;
	} while (false);

	return error_no;
}

int if_write(fs_request_t *req)
{
	int error_no;
	struct _Twrite *request = &(req->packet.Twrite);
	do {
		session_t *session = session_find_by_request(req);
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file;
		file = session_find_file(session, request->fid);
		if (!file
				|| (file->f_flag == O_RDONLY)
				|| (file->f_flag == O_ACCMODE)) {
			error_no = EBADF;
			break;
		}

		fsmsg_t *message = &(req->packet);
		unsigned int pos = 0;
		ER_UINT result = 0;
		for (size_t rest = message->Twrite.count; rest > 0;) {
			size_t len = (rest > sizeof(line)) ?
					sizeof(line) : rest;
			if (kcall->region_get(
					thread_id_of_token(message->header.token),
					(char *) ((unsigned int) (message->Twrite.data) + pos),
					len,
					line)) {
				result = E_SYS;
				break;
			} else {
				result = _write(file, message->Twrite.offset,
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

		//TODO return Rerror if result is error
		//message->header.token = message->head.token;
		message->header.type = Rwrite;
		//message->Rwrite.tag = message->Twrite.tag;
		message->Rwrite.count = result;
		reply(req, MESSAGE_SIZE(Rwrite));
		return 0;
	} while (false);

	return error_no;
}

static ER_UINT _write(struct file *file, const UW start, const UW size,
		const char *inbuf)
{
	//TODO select device from file->f_channel
	//TODO select target from file->session->window

	int result = kcall->mutex_lock(cons_mid, TMO_FEVR);
	if (result)
		kcall->printk("hmi: main cannot lock %d\n", result);
	else {
		mouse_hide();
		terminal_write((char *) inbuf, &state0, 0, size);
		mouse_show();
		result = kcall->mutex_unlock(cons_mid);
		if (result)
			kcall->printk("hmi: main cannot unlock %d\n", result);
	}

	return size;
}
