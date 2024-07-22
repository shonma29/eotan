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
#include "console.h"
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

//		if (request->nwname)
//TODO check if root?
//			if ((parent->f_vnode->mode & S_IFMT) != S_IFDIR) {
//				error_no = ENOTDIR;
//				break;
//			}

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

		driver_t *driver = NULL;
		if (request->nwname) {
			error_no = session_get_path(req->buf, session,
					unpack_tid(req), request->wname);
			if (error_no) {
				if (file)
					session_destroy_file(session, newfid);

				break;
			}

			char *p = req->buf;
			if (*p == '/')
				p++;

			driver = device_lookup(p);
			if (!driver) {
				if (file)
					session_destroy_file(session, newfid);

				error_no = ENOENT;
				break;
			}
		} else {
			//TODO parent.refer_count++ ?
		}

		if (!file) {
			//TODO really?
			file = parent;
		}

		file->f_flag = O_ACCMODE;
		file->size = 0;
		file->driver = driver;

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
