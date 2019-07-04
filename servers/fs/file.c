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


int if_open(fs_request *req)
{
	session_t *session = session_find(unpack_sid(req));
	if (!session)
		return ESRCH;

	vnode_t *starting_node;
	int error_no = session_get_path(req->buf, &starting_node,
			session, unpack_tid(req), (char*)(req->packet.arg1));
	if (error_no)
		return error_no;

	vnode_t *vnode;
	//TODO mode is not needed in plan9?
	error_no = vfs_open(starting_node, req->buf, req->packet.arg2,
			req->packet.arg3, &(session->permission), &vnode);
	if (error_no)
		return error_no;

	struct file *file;
	error_no = session_create_desc(&file, session, -1);
	if (error_no) {
		vnodes_remove(vnode);
		return error_no;
	}

	file->f_vnode = vnode;
	file->f_flag = req->packet.arg2 & O_ACCMODE;

	reply2(req->rdvno, 0, file->node.key, 0);
	return 0;
}

int if_close(fs_request *req)
{
	int error_no;
	devmsg_t *request = (devmsg_t*)&(req->packet);
	session_t *session = session_find(unpack_sid(req));
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

	return 0;
}

int if_read(fs_request *req)
{
	devmsg_t *request = (devmsg_t*)&(req->packet);
	session_t *session = session_find(unpack_sid(req));
	if (!session) {
		reply_dev_error(req->rdvno, request->Tread.tag, ESRCH);
		return 0;
	}

	struct file *file = session_find_desc(session, request->Tread.fid);
	if (!file) {
		reply_dev_error(req->rdvno, request->Tread.tag, EBADF);
		return 0;
	}

	if (file->f_flag == O_WRONLY) {
		reply_dev_error(req->rdvno, request->Tread.tag, EBADF);
		return 0;
	}

	copier_t copier = {
		copy_to_user,
		request->Tread.data,
		unpack_tid(req)
	};
	size_t count = 0;
	int error_no = fs_read(file->f_vnode, &copier, request->Tread.offset,
			request->Tread.count, &count);
	if (error_no) {
		reply_dev_error(req->rdvno, request->Tread.tag,
				error_no);
		return 0;
	}

	devmsg_t response;
	response.type = Rread;
	response.Rread.tag = req->packet.process_id;
	response.Rread.count = count;
	reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rread));
	return 0;
}

int if_write(fs_request *req)
{
	devmsg_t *request = (devmsg_t*)&(req->packet);
	session_t *session = session_find(unpack_sid(req));
	if (!session) {
		reply_dev_error(req->rdvno, request->Twrite.tag, ESRCH);
		return 0;
	}

	struct file *file;
	file = session_find_desc(session, request->Twrite.fid);
	if (!file) {
		reply_dev_error(req->rdvno, request->Twrite.tag, EBADF);
		return 0;
	}

	if (file->f_flag == O_RDONLY) {
		reply_dev_error(req->rdvno, request->Twrite.tag, EBADF);
		return 0;
	}

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
				offset - file->f_vnode->size, &wrote_size);
	}
	if (error_no) {
		reply_dev_error(req->rdvno, request->Twrite.tag, error_no);
		return 0;
	}

	copier_t copier = {
		copy_from_user,
		request->Twrite.data,
		unpack_tid(req)
	};
	size_t wrote_size;
	error_no = vfs_write(file->f_vnode, &copier,
			offset, request->Twrite.count, &wrote_size);
	if (error_no) {
		reply_dev_error(req->rdvno, request->Twrite.tag, error_no);
		return 0;
	}

	devmsg_t response;
	response.type = Rwrite;
	response.Rwrite.tag = req->packet.process_id;
	response.Rwrite.count = wrote_size;
	reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rwrite));
	return 0;
}

int copy_from_user(void *dest, void *src, const size_t len)
{
	copier_t *cp = (copier_t*)src;
	int error_no = kcall->region_get(cp->caller, cp->buf, len, dest);
	if (error_no)
		return error_no;

	cp->buf += len;
	return 0;
}

int copy_to_user(void *dest, void *src, const size_t len)
{
	copier_t *cp = (copier_t*)dest;
	int error_no = kcall->region_put(cp->caller, cp->buf, len, src);
	if (error_no)
		return error_no;

	cp->buf += len;
	return 0;
}
