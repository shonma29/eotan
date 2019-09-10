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
#include "api.h"
#include "session.h"


void if_walk(fs_request *req)
{
	int error_no;
	struct _Twalk *request = &(req->packet.Twalk);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		int fid = request->fid;
		struct file *parent = session_find_file(session, fid);
		if (!parent) {
			error_no = EBADF;
			break;
		}

		if (parent->f_flag != O_ACCMODE) {
			error_no = EBADF;
			break;
		}

		if (request->nwname)
			if ((parent->f_vnode->mode & S_IFMT) != S_IFDIR) {
				error_no = ENOTDIR;
				break;
			}

		struct file *file = NULL;
		int newfid = request->newfid;
		if (newfid == fid) {
			if (request->nwname == 0) {
				//TODO really?
				fsmsg_t response;
				response.header.token =
						req->packet.header.token;
				response.header.type = Rwalk;
				response.Rwalk.tag = request->tag;
				//TODO return nwqid and wqid
				reply(req->tag, &response,
						MESSAGE_SIZE(Rwalk));
				return;
			}
		} else {
			error_no = session_create_file(&file, session, newfid);
			if (error_no)
				break;
		}

		vnode_t *vnode;
		if (request->nwname) {
			vnode_t *starting_node;
			error_no = session_get_path(req->buf, &starting_node,
					session, parent->f_vnode,
					unpack_tid(req), request->wname);
			if (error_no) {
				if (file)
					session_destroy_file(session, newfid);

				break;
			}

			//TODO '.' entry is illegal
			//TODO '..' cannot use in top level
			error_no = vfs_walk(starting_node, req->buf, O_RDONLY,
					&(session->permission), &vnode);
			if (error_no) {
				if (file)
					session_destroy_file(session, newfid);

				break;
			}
		} else {
			vnode = parent->f_vnode;
			vnode->refer_count++;
		}

		if (!file) {
			vnodes_remove(parent->f_vnode);
			file = parent;
		}

		file->f_vnode = vnode;
		file->f_flag = O_ACCMODE;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rwalk;
		response.Rwalk.tag = request->tag;
		//TODO return nwqid and wqid
		reply(req->tag, &response, MESSAGE_SIZE(Rwalk));
		return;
	} while (false);

	//TODO return nwqid and wqid
	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void if_create(fs_request *req)
{
	int error_no;
	struct _Tcreate *request = &(req->packet.Tcreate);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		struct file *parent = session_find_file(session, request->fid);
		if (!parent) {
			error_no = EBADF;
			break;
		}

		if (parent->f_flag != O_ACCMODE) {
			error_no = EBADF;
			break;
		}

		if ((parent->f_vnode->mode & S_IFMT) != S_IFDIR) {
			error_no = ENOTDIR;
			break;
		}

		//TODO check filename. not path
		vnode_t *starting_node;
		error_no = session_get_path(req->buf, &starting_node,
				session, parent->f_vnode, unpack_tid(req),
				request->name);
		if (error_no)
			break;

		vnode_t *vnode;
		error_no = vfs_create(starting_node, req->buf, request->mode,
				request->perm, &(session->permission), &vnode);
		if (error_no)
			break;

		vnodes_remove(parent->f_vnode);
		parent->f_vnode = vnode;
		//TODO really?
		parent->f_flag = request->mode & O_ACCMODE;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rcreate;
		response.Rcreate.tag = request->tag;
		response.Rcreate.qid = parent->node.key;
		response.Rcreate.iounit = 0;
		reply(req->tag, &response, MESSAGE_SIZE(Rcreate));
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void if_remove(fs_request *req)
{
	int error_no;
	struct _Tremove *request = &(req->packet.Tremove);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = EPERM;
			break;
		}

		int fid = request->fid;
		struct file *file = session_find_file(session, fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		error_no = vfs_remove(file->f_vnode, &(session->permission));

		int e = session_destroy_file(session, fid);
		if (e) {
			//TODO what to do?
		}


		if (error_no)
			break;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rremove;
		response.Rremove.tag = request->tag;
		reply(req->tag, &response, MESSAGE_SIZE(Rremove));
		return;
	} while (false);

	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}
