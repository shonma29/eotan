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
#include <core/options.h>
#include <sys/errno.h>
#include "api.h"
#include "session.h"
#include "../../lib/libserv/libserv.h"


int if_walk(fs_request *req)
{
	devmsg_t *request = (devmsg_t*)&(req->packet);
	//TODO set adequate errno
	int error_no;
//log_info("walk-1 %x\n", req->rdvno);
	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		int fid = request->Twalk.fid;
		struct file *parent = session_find_desc(session, fid);
		if (!parent) {
//log_info("walk0 %d\n", fid);
			error_no = EBADF;
			break;
		}

		if (parent->f_flag != O_ACCMODE) {
//log_info("walk1\n");
			error_no = EBADF;
			break;
		}

		struct file *file = NULL;
		int newfid = request->Twalk.newfid;
		if (request->Twalk.nwname) {
//log_info("walk3\n");
			if ((parent->f_vnode->mode & S_IFMT) != S_IFDIR) {
				error_no = ENOTDIR;
//log_info("walk4\n");
				break;
			}

			if (fid != newfid) {
//log_info("walk6\n");
				error_no = session_create_desc(&file, session,
						newfid);
				if (error_no) {
//log_info("walk7\n");
					break;
				}
			}
		} else if (newfid == fid) {
//log_info("walk9\n");
			//TODO really?
			devmsg_t response;
			response.type = Rwalk;
			response.Rwalk.tag = request->Twalk.tag;
			//TODO return nwqid and wqid
			reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rwalk));
			return 0;
		}

		vnode_t *vnode;
		if (request->Twalk.nwname) {
//log_info("walk11\n");
			vnode_t *starting_node;
			error_no = session_get_path(req->buf, &starting_node,
					session, parent->f_vnode,
					unpack_tid(req), request->Twalk.wname);
			if (error_no) {
//log_info("walk12\n");
				if (file) {
//log_info("walk13\n");
					session_destroy_desc(session, newfid);
				}

				break;
			}

			//TODO '.' entry is illegal
			//TODO '..' cannot use in top level
			error_no = vfs_walk(starting_node, req->buf, O_RDONLY,
					&(session->permission), &vnode);
			if (error_no) {
//log_info("walk15\n");
				if (file) {
//log_info("walk16\n");
					session_destroy_desc(session, newfid);
				}

				break;
			}
		} else {
//log_info("walk17\n");
			vnode = parent->f_vnode;
			vnode->refer_count++;
		}

		if (!file) {
//log_info("walk18\n");
			vnodes_remove(parent->f_vnode);
			file = parent;
		}

		file->f_vnode = vnode;
		file->f_flag = O_ACCMODE;

		devmsg_t response;
		response.type = Rwalk;
		response.Rwalk.tag = request->Twalk.tag;
		//TODO return nwqid and wqid
//log_info("walk22 %x %p %d\n", req->rdvno, &response, MESSAGE_SIZE(Rwalk));
//		int result = 
		reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rwalk));
//log_info("walk23 %d\n", result);
		return 0;
	} while (false);
//log_info("walk20\n");
	//TODO return nwqid and wqid
	reply_dev_error(req->rdvno, request->Twalk.tag, error_no);
	return 0;
}

int if_create(fs_request *req)
{
	session_t *session = session_find(unpack_sid(req));
	if (!session)
		return ESRCH;

	struct file *parent = session_find_desc(session, req->packet.arg5);
	if (!parent)
		return EBADF;

	if (parent->f_flag != O_ACCMODE)
		return EBADF;

	if ((parent->f_vnode->mode & S_IFMT) != S_IFDIR)
		return ENOTDIR;

	//TODO lock fid
	vnode_t *starting_node;
	int error_no = session_get_path(req->buf, &starting_node, session,
			parent->f_vnode, unpack_tid(req),
			(char*)(req->packet.arg1));
	if (error_no)
		return error_no;

	vnode_t *vnode;
	error_no = vfs_create(starting_node, req->buf, req->packet.arg2,
			req->packet.arg3, &(session->permission), &vnode);
	if (error_no)
		return error_no;

	int fid = req->packet.arg4;
	struct file *file;
	error_no = session_create_desc(&file, session, fid);
	if (error_no) {
		vnodes_remove(vnode);
		return error_no;
	}

	file->f_vnode = vnode;
	//TODO really?
	file->f_flag = req->packet.arg2 & O_ACCMODE;

	reply2(req->rdvno, 0, file->node.key, 0);
	return 0;
}

int if_remove(fs_request *req)
{
	devmsg_t *request = (devmsg_t*)&(req->packet);
	int error_no;

	do {
		session_t *session = session_find(unpack_sid(req));
		if (!session) {
			error_no = ESRCH;
			break;
		}

		int fid = request->Tremove.fid;
		struct file *file = session_find_desc(session, fid);
		if (!file) {
			error_no = EBADF;
			break;
		}

		error_no = vfs_remove(file->f_vnode, &(session->permission));

		int e = session_destroy_desc(session, fid);
		if (e) {
			//TODO what to do?
		}


		if (error_no)
			break;

		devmsg_t response;
		response.type = Rremove;
		response.Rremove.tag = request->Tremove.tag;
		reply_dev(req->rdvno, &response, MESSAGE_SIZE(Rremove));
		return 0;
	} while (false);

	reply_dev_error(req->rdvno, request->Tremove.tag, error_no);
	return 0;
}
