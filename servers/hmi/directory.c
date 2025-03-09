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
#include <nerve/kcall.h>
#include "hmi.h"
#include "api.h"
#include "session.h"

static char path_buf[PATH_MAX];

static int _get_path(char *, const int, const char *);


int if_walk(fs_request_t *req)
{
	int error_no;
	struct _Twalk *request = &(req->packet.Twalk);
	do {
		session_t *session = session_find_by_request(req);
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
			if (parent->f_channel) {
				error_no = ENOTDIR;
				break;
			}

		struct file *file = NULL;
		int newfid = request->newfid;
		if (newfid == fid) {
			if (request->nwname == 0) {
				//TODO really?
				fsmsg_t *response = &(req->packet);
				//response->header.token =
				//		req->packet.header.token;
				response->header.type = Rwalk;
				//response->Rwalk.tag = request->tag;
				//TODO return nwqid and wqid
				reply(req, MESSAGE_SIZE(Rwalk));
				return 0;
			}
		} else {
			error_no = session_create_file(&file, session, newfid);
			if (error_no)
				break;
		}

		driver_t *driver = NULL;
		if (request->nwname) {
			error_no = _get_path(path_buf, unpack_tid(req),
					request->wname);
			if (error_no) {
				if (file)
					session_destroy_file(session, newfid);

				break;
			}

			char *p = path_buf;
			//TODO support walk '/'
			if (*p == '/')
				p++;

			driver = device_lookup(p);
			if (!driver) {
				if (file)
					session_destroy_file(session, newfid);

				error_no = ENOENT;
				break;
			}
		}

		if (!file)
			file = parent;

		file->f_flag = O_ACCMODE;

		if (driver)
			file->f_channel = driver->channel;

		fsmsg_t *response = &(req->packet);
		//response->header.token = req->packet.header.token;
		response->header.type = Rwalk;
		//response->Rwalk.tag = request->tag;
		//TODO return nwqid and wqid
		reply(req, MESSAGE_SIZE(Rwalk));
		return 0;
	} while (false);

	//TODO return nwqid and wqid
	return error_no;
}

static int _get_path(char *dest, const int tid, const char *src)
{
	ER_UINT len = kcall->region_copy(tid, src, PATH_MAX + 1, dest);
	if (len <= 0)
		return EFAULT;

	if (len > PATH_MAX)
		return ENAMETOOLONG;

	return 0;
}