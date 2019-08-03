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
#include "api.h"
#include "session.h"


int if_stat(fs_request *req)
{
	int error_no = 0;
	devmsg_t *request = (devmsg_t*)&(req->packet);

	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		struct file *file = session_find_desc(session,
				request->Tstat.fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		struct stat *st = (struct stat*)&(req->buf);
		int error_no = vfs_stat(file->f_vnode, st);
		if (error_no)
			break;

		if (kcall->region_put(unpack_tid(req),
				request->Tstat.stat, sizeof(*st), st)) {
			error_no = EFAULT;
			break;
		}

		devmsg_t response;
		response.type = Rstat;
		response.Rstat.tag = request->Tstat.tag;
		reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rstat));
		return 0;
	} while (false);

	reply_dev_error(req->rdvno, request->Tstat.tag, error_no);
	return 0;
}

int if_wstat(fs_request *req)
{
	int error_no = 0;
	devmsg_t *request = (devmsg_t*)&(req->packet);

	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		struct file *file = session_find_desc(session,
				request->Twstat.fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		//TODO allow group leader
		vnode_t *vnode = file->f_vnode;
		if (vnode->uid != session->permission.uid) {
			error_no = EPERM;
			break;
		}

		struct stat *st = (struct stat*)&(req->buf);
		if (kcall->region_get(unpack_tid(req),
				request->Twstat.stat, sizeof(*st), st)) {
			error_no = EFAULT;
			break;
		}

		if (st->st_mode & UNMODIFIABLE_MODE_BITS) {
			error_no = EINVAL;
			break;
		}

		vnode->mode = (vnode->mode & S_IFMT) | st->st_mode;
		error_no = vfs_wstat(vnode);
		if (error_no)
			break;

		devmsg_t response;
		response.type = Rwstat;
		response.Rwstat.tag = request->Twstat.tag;
		reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rwstat));
		return 0;
	} while (false);

	reply_dev_error(req->rdvno, request->Twstat.tag, error_no);
	return 0;
}
