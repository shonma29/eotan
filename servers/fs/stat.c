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


void if_stat(fs_request *req)
{
	int error_no;
	struct _Tstat *request = &(req->packet.Tstat);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		struct stat *st = (struct stat*)&(req->buf);
		error_no = vfs_stat(file->f_vnode, st);
		if (error_no)
			break;

		if (kcall->region_put(unpack_tid(req), request->stat,
				sizeof(*st), st)) {
			error_no = EFAULT;
			break;
		}

		devmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rstat;
		response.Rstat.tag = request->tag;
		reply(req->rdvno, &response, MESSAGE_SIZE(Rstat));
		return;
	} while (false);

	reply_error(req->rdvno, req->packet.header.token, request->tag,
			error_no);
}

void if_wstat(fs_request *req)
{
	int error_no;
	struct _Twstat *request = &(req->packet.Twstat);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		struct file *file = session_find_file(session, request->fid);
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
		if (kcall->region_get(unpack_tid(req), request->stat,
				sizeof(*st), st)) {
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
		response.header.token = req->packet.header.token;
		response.header.type = Rwstat;
		response.Rwstat.tag = request->tag;
		reply(req->rdvno, &response, MESSAGE_SIZE(Rwstat));
		return;
	} while (false);

	reply_error(req->rdvno, req->packet.header.token, request->tag,
			error_no);
}
