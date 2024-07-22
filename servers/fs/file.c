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
#include <string.h>
#include <sys/errno.h>
#include "../../lib/libserv/libserv.h"
#include "api.h"
#include "fs.h"
#include "session.h"


void if_open(fs_request *req)
{
	int error_no;
	struct _Topen *request = &(req->packet.Topen);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		if (file->f_flag != O_ACCMODE) {
			error_no = EBUSY;
			break;
		}

		//TODO check mode only here
		error_no = vfs_open(file->f_vnode, request->mode,
				&(session->permission));
		if (error_no)
			break;

		//TODO save other bits
		file->f_flag = request->mode & O_ACCMODE;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Ropen;
		response.Ropen.tag = request->tag;
		response.Ropen.qid = file->f_vnode->index;
		response.Ropen.iounit = 0;
		reply(req->tag, &response, MESSAGE_SIZE(Ropen));
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void if_clunk(fs_request *req)
{
	int error_no;
	struct _Tclunk *request = &(req->packet.Tclunk);
	do {
		session_t *session = session_find(unpack_sid(req));
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
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void if_read(fs_request *req)
{
	int error_no;
	struct _Tread *request = &(req->packet.Tread);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		if (file->f_flag == O_WRONLY) {
			error_no = EBADF;
			break;
		}

		if (request->offset < 0) {
			error_no = EINVAL;
			break;
		}

		copier_t copier = {
			copy_to_user,
			request->data,
			unpack_tid(req)
		};
		size_t count;
		error_no = fs_read(file->f_vnode, &copier, request->offset,
				request->count, &count);
		if (error_no)
			break;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rread;
		response.Rread.tag = request->tag;
		response.Rread.count = count;
		reply(req->tag, &response, MESSAGE_SIZE(Rread));
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void if_write(fs_request *req)
{
	int error_no;
	struct _Twrite *request = &(req->packet.Twrite);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *file;
		file = session_find_file(session, request->fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		if (file->f_flag == O_RDONLY) {
			error_no = EBADF;
			break;
		}

		if (request->offset < 0) {
			error_no = EINVAL;
			break;
		}

		size_t wrote_size;
		if (request->count) {
			error_no = 0;
			unsigned int offset = request->offset;
			//TODO move to vfs
			if (offset > file->f_vnode->size) {
				memset(req->buf, 0, sizeof(req->buf));
				//TODO when length > size of buf?
				copier_t copier = {
					copy_from,
					req->buf
				};
				size_t wrote_size;
				error_no = vfs_write(file->f_vnode, &copier,
						file->f_vnode->size,
						offset - file->f_vnode->size,
						&wrote_size);
			}
			if (error_no)
				break;

			copier_t copier = {
				copy_from_user,
				request->data,
				unpack_tid(req)
			};
			error_no = vfs_write(file->f_vnode, &copier,
					offset, request->count, &wrote_size);
			if (error_no)
				break;
		} else
			wrote_size = 0;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rwrite;
		response.Rwrite.tag = request->tag;
		response.Rwrite.count = wrote_size;
		reply(req->tag, &response, MESSAGE_SIZE(Rwrite));
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}
