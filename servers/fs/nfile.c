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
#include <sys/unistd.h>
#include "api.h"
#include "session.h"


int if_open(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
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
	file->f_count = 0;
	file->f_offset = (req->packet.arg2 & O_APPEND)? vnode->size:0;

	reply2(req->rdvno, 0, file->node.key, 0);
	return 0;
}

int if_close(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	int error_no = session_destroy_desc(session, req->packet.arg1);
	if (error_no)
		return error_no;

	reply2(req->rdvno, 0, 0, 0);
	return 0;
}

int if_lseek(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	struct file *file = session_find_desc(session, req->packet.arg1);
	if (!file)
		return EBADF;

	off_t *offset = (off_t*)&(req->packet.arg2);
	off_t next = *offset;
	if (next < 0)
		return EINVAL;

	switch (req->packet.arg4) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
	{
		//TODO off_t size is ambiguous. find another way
		off_t rest = LLONG_MAX - next;
		if (file->f_offset > rest)
			return EOVERFLOW;
	}

		next += file->f_offset;
		break;
	case SEEK_END:
	{
		//TODO off_t size is ambiguous. find another way
		off_t rest = LLONG_MAX - next;
		if (file->f_vnode->size > rest)
			return EOVERFLOW;
	}

		next += file->f_vnode->size;
		break;
	default:
		return EINVAL;
	}

	if (file->f_vnode->mode & S_IFCHR)
		if (file->f_offset > file->f_vnode->size)
			return EINVAL;

	file->f_offset = next;
	reply64(req->rdvno, 0, next);
	return 0;
}

int if_dup2(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	struct file *src = session_find_desc(session, req->packet.arg1);
	if (!src)
		return EBADF;

	struct file *dest = session_find_desc(session, req->packet.arg2);
	if (dest) {
		int error_no = vnodes_remove(dest->f_vnode);
		if (error_no)
			return error_no;
	} else {
		int error_no = session_create_desc(&dest, session,
				req->packet.arg2);
		if (error_no)
			return error_no;
	}

	//TODO share file struct, and increment count
	src->f_vnode->refer_count++;
	dest->f_vnode = src->f_vnode;
	dest->f_flag = src->f_flag & O_ACCMODE;
	dest->f_count = 0;
	dest->f_offset = (src->f_flag & O_APPEND)? src->f_vnode->size:0;

	reply2(req->rdvno, 0, req->packet.arg2, 0);
	return 0;
}
