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
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "fs.h"
#include "session.h"

static int path2vnode(vnode_t **vnode, const session_t *session, const int tid,
		const unsigned char *path, unsigned char *buf);


static int path2vnode(vnode_t **vnode, const session_t *session, const int tid,
		const unsigned char *path, unsigned char *buf)
{
	vnode_t *starting_node;
	int error_no = session_get_path(buf, &starting_node, session, tid,
			(unsigned char*)path);
	if (error_no)
		return error_no;

	return vfs_walk(starting_node, (char*)buf, O_ACCMODE,
			&(session->permission), vnode);
}

int if_chmod(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	vnode_t *vnode;
	int error_no = path2vnode(&vnode, session, unpack_tid(req),
			(unsigned char*)(req->packet.arg1),
			(unsigned char*)(req->buf));
	if (error_no)
		return error_no;

	//TODO allow group leader
	if (vnode->uid != session->permission.uid)
		return EPERM;

	if (req->packet.arg2 & UNMODIFIABLE_MODE_BITS)
		return EINVAL;

	vnode->mode = (vnode->mode & S_IFMT) | req->packet.arg2;
	error_no = vfs_wstat(vnode);
	vnodes_remove(vnode);

	if (!error_no)
		reply2(req->rdvno, 0, 0, 0);

	return error_no;
}

int if_fstat(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	struct file *file = session_find_desc(session, req->packet.arg1);
	if (!file)
		return EBADF;

	struct stat *st = (struct stat*)&(req->buf);
	int error_no = vfs_stat(file->f_vnode, st);
	if (error_no)
		return error_no;

	if (kcall->region_put(unpack_tid(req),
			(struct stat*)(req->packet.arg2), sizeof(*st), st))
		return EFAULT;

	reply2(req->rdvno, 0, 0, 0);
	return 0;
}
