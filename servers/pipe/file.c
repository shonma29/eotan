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
#include <fcntl.h>
#include <sys/errno.h>
#include <fs/vfs.h>
#include <nerve/kcall.h>
#include "api.h"
#include "session.h"

#define MASK_UNSUPPORTED_MODE (O_APPEND | O_TRUNC)

static bool _read(pipe_channel_t *);
static int _put(pipe_channel_t *, fs_request_t *, const int);
static bool _write(pipe_channel_t *);
static int _get(pipe_channel_t *, fs_request_t *, const int);


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
		file->f_channel->refer_count++;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Ropen;
		response.Ropen.tag = request->tag;
		response.Ropen.qid = file->node.key;
		response.Ropen.iounit = 0;
		reply(req->tag, &response, MESSAGE_SIZE(Ropen));
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

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rclunk;
		response.Rclunk.tag = request->tag;
		reply(req->tag, &response, MESSAGE_SIZE(Rclunk));
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

		if (request->count) {
			pipe_channel_t *channel = file->f_channel;
			if (!enqueue_request(&(channel->readers), req)) {
				error_no = ENOBUFS;
				break;
			}

			bool processed;
			do {
				processed = _read(channel);
				if (processed)
					processed = _write(channel);
			} while (processed);
		} else {
			req->position = 0;
			reply_read(req);
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

		pipe_channel_t *channel = file->f_channel;
		if (request->count
				&& (channel->partner->refer_count > 0)) {
			if (!enqueue_request(&(channel->partner->writers), req)) {
				error_no = ENOBUFS;
				break;
			}

			bool processed;
			do {
				processed = _write(channel->partner);
				if (processed)
					processed = _read(channel->partner);
			} while (processed);
		} else {
			req->position = 0;
			reply_write(req);
		}

		return 0;
	} while (false);

	return error_no;
}

static bool _read(pipe_channel_t *p)
{
	bool has_read = false;
	for (list_t *head;
			!list_is_edge(&(p->readers),
					head = list_next(&(p->readers)));) {
		fs_request_t *req = (fs_request_t *) getRequestFromList(head);
		if (p->write_position < p->read_position) {
			size_t rest = req->packet.Tread.count - req->position;
			int len = p->size - p->read_position;
			if (rest <= len) {
				if (_put(p, req, rest))
					continue;

				has_read = true;
				reply_read(req);
				dequeue_request(req);
				continue;
			} else {
				if (_put(p, req, len))
					continue;

				has_read = true;
			}
		}

		if (p->read_position < p->write_position) {
			size_t rest = req->packet.Tread.count - req->position;
			int len = p->write_position - p->read_position;
			if (rest <= len) {
				if (_put(p, req, rest))
					continue;

				has_read = true;
				reply_read(req);
				dequeue_request(req);
				continue;
			} else {
				if (_put(p, req, len))
					continue;

				has_read = true;
			}
		}

		if (p->partner->refer_count <= 0) {
			reply_read(req);
			dequeue_request(req);
		}

		break;
	}

	return has_read;
}

static int _put(pipe_channel_t *p, fs_request_t *req, const int len)
{
	int result = kcall->region_put(req->tid,
			&(req->packet.Tread.data[req->position]), len,
			&(p->buf[p->read_position]));
	if (result) {
		//TODO send kill
		reply_error(req->tag, req->packet.header.token,
				req->packet.Tread.tag, EFAULT);
		dequeue_request(req);
		return result;
	}

	req->position += len;
	p->read_position = (p->read_position + len) & p->position_mask;
	return 0;
}

static bool _write(pipe_channel_t *p)
{
	bool has_written = false;
	for (list_t *head;
			!list_is_edge(&(p->writers),
					head = list_next(&(p->writers)));) {
		fs_request_t *req = (fs_request_t *) getRequestFromList(head);
		if (p->read_position <= p->write_position) {
			size_t rest = req->packet.Twrite.count - req->position;
			int len = p->size - p->write_position;
			if (!(p->read_position))
				--len;

			if (rest <= len) {
				if (_get(p, req, rest))
					continue;

				has_written = true;
				reply_write(req);
				dequeue_request(req);
				continue;
			} else {
				if (_get(p, req, len))
					continue;

				has_written = true;
			}
		}

		if ((1 < p->read_position)
				&& (p->write_position < p->read_position - 1)) {
			size_t rest = req->packet.Twrite.count - req->position;
			int len = p->read_position - 1 - p->write_position;
			if (rest <= len) {
				if (_get(p, req, rest))
					continue;

				has_written = true;
				reply_write(req);
				dequeue_request(req);
				continue;
			} else {
				if (_get(p, req, len))
					continue;

				has_written = true;
			}
		}

		break;
	}

	return has_written;
}

static int _get(pipe_channel_t *p, fs_request_t *req, const int len)
{
	int result = kcall->region_get(req->tid,
			&(req->packet.Twrite.data[req->position]), len,
			&(p->buf[p->write_position]));
	if (result) {
		//TODO send kill
		reply_error(req->tag, req->packet.header.token,
				req->packet.Twrite.tag, EFAULT);
		dequeue_request(req);
		return result;
	}

	req->position += len;
	p->write_position = (p->write_position + len) & p->position_mask;
	return 0;
}
