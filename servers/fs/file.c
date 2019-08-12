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
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "api.h"
#include "session.h"

static int copy_from_user(void *, void *, const size_t);
static int copy_to_user(void *, void *, const size_t);


void if_open(fs_request *req)
{
	devmsg_t *request = &(req->packet);
	session_t *session = session_find(unpack_sid(request->Topen.tag));
	if (!session) {
		reply_dev_error(req->rdvno, request->Topen.tag, ESRCH);
		return;
	}

	struct file *file = session_find_desc(session, request->Topen.fid);
	if (!file) {
		reply_dev_error(req->rdvno, request->Topen.tag, EBADF);
		return;
	}

	if (file->f_flag != O_ACCMODE) {
		reply_dev_error(req->rdvno, request->Topen.tag, EBUSY);
		return;
	}

	//TODO check mode only here
	int error_no = vfs_open(file->f_vnode, request->Topen.mode,
			&(session->permission));
	if (error_no) {
		reply_dev_error(req->rdvno, request->Topen.tag, error_no);
		return;
	}

	//TODO save other bits
	file->f_flag = request->Topen.mode & O_ACCMODE;

	devmsg_t response;
	response.type = Ropen;
	response.Ropen.tag = request->Topen.tag;
	response.Ropen.qid = file->f_vnode->index;
	response.Ropen.iounit = 0;
	reply_dev(req->rdvno, &response, MESSAGE_SIZE(Ropen));
}

void if_clunk(fs_request *req)
{
	int error_no;
	devmsg_t *request = &(req->packet);
	session_t *session = session_find(unpack_sid(request->Tclunk.tag));
	if (session)
		error_no = session_destroy_desc(session, request->Tclunk.fid);
	else
		error_no = ESRCH;

	if (error_no)
		reply_dev_error(req->rdvno, request->Tclunk.tag, error_no);
	else {
		devmsg_t response;
		response.type = Rclunk;
		response.Rclunk.tag = request->Tclunk.tag;
		reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rclunk));
	}
}

void if_read(fs_request *req)
{
	devmsg_t *request = &(req->packet);
	session_t *session = session_find(unpack_sid(request->Tread.tag));
	if (!session) {
		reply_dev_error(req->rdvno, request->Tread.tag, ESRCH);
		return;
	}

	struct file *file = session_find_desc(session, request->Tread.fid);
	if (!file) {
		reply_dev_error(req->rdvno, request->Tread.tag, EBADF);
		return;
	}

	if (file->f_flag == O_WRONLY) {
		reply_dev_error(req->rdvno, request->Tread.tag, EBADF);
		return;
	}

	if (request->Tread.offset < 0) {
		reply_dev_error(req->rdvno, request->Tread.tag, EINVAL);
		return;
	}

	copier_t copier = {
		copy_to_user,
		request->Tread.data,
		unpack_tid(request->Tread.tag)
	};
	size_t count;
	int error_no = fs_read(file->f_vnode, &copier, request->Tread.offset,
			request->Tread.count, &count);
	if (error_no) {
		reply_dev_error(req->rdvno, request->Tread.tag, error_no);
		return;
	}

	devmsg_t response;
	response.type = Rread;
	response.Rread.tag = request->Tread.tag;
	response.Rread.count = count;
	reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rread));
}

void if_write(fs_request *req)
{
	devmsg_t *request = &(req->packet);
	session_t *session = session_find(unpack_sid(request->Twrite.tag));
	if (!session) {
		reply_dev_error(req->rdvno, request->Twrite.tag, ESRCH);
		return;
	}

	struct file *file;
	file = session_find_desc(session, request->Twrite.fid);
	if (!file) {
		reply_dev_error(req->rdvno, request->Twrite.tag, EBADF);
		return;
	}

	if (file->f_flag == O_RDONLY) {
		reply_dev_error(req->rdvno, request->Twrite.tag, EBADF);
		return;
	}

	if (request->Twrite.offset < 0) {
		reply_dev_error(req->rdvno, request->Twrite.tag, EINVAL);
		return;
	}

	size_t wrote_size;
	if (request->Twrite.count) {
		int error_no = 0;
		unsigned int offset = request->Twrite.offset;
		if (offset > file->f_vnode->size) {
			memset(req->buf, 0, sizeof(req->buf));
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
		if (error_no) {
			reply_dev_error(req->rdvno, request->Twrite.tag,
					error_no);
			return;
		}

		copier_t copier = {
			copy_from_user,
			request->Twrite.data,
			unpack_tid(request->Twrite.tag)
		};
		error_no = vfs_write(file->f_vnode, &copier,
				offset, request->Twrite.count, &wrote_size);
		if (error_no) {
			reply_dev_error(req->rdvno, request->Twrite.tag,
					error_no);
			return;
		}
	} else
		wrote_size = 0;

	devmsg_t response;
	response.type = Rwrite;
	response.Rwrite.tag = request->Twrite.tag;
	response.Rwrite.count = wrote_size;
	reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rwrite));
}

static int copy_from_user(void *dest, void *src, const size_t len)
{
	copier_t *cp = (copier_t*)src;
	int error_no = kcall->region_get(cp->caller, cp->buf, len, dest);
	if (error_no)
		return error_no;

	cp->buf += len;
	return 0;
}

static int copy_to_user(void *dest, void *src, const size_t len)
{
	copier_t *cp = (copier_t*)dest;
	int error_no = kcall->region_put(cp->caller, cp->buf, len, src);
	if (error_no)
		return error_no;

	cp->buf += len;
	return 0;
}
