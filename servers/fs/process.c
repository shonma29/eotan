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
#include <errno.h>
#include "api.h"
#include "session.h"
#include "procfs/process.h"


int if_fork(fs_request *req)
{
	session_t *session = session_find(unpack_sid(req));
	if (!session)
		return ESRCH;

	session->cwd->refer_count++;
	reply2(req->rdvno, 0, 0, 0);
	return 0;
}

int if_exec(fs_request *req)
{
	session_t *session = session_find(unpack_sid(req));
	if (!session)
		return ESRCH;

	vnode_t *parent;
	int error_no = session_get_path(req->buf, &parent, session,
			unpack_tid(req), (char*)(req->packet.arg1));
	if (error_no)
		return error_no;

	error_no = exec_program(&(req->packet), session, parent, req->buf);
	if (error_no) {
		//TODO release resource if needed
		//session_destroy(session);
		return error_no;
	}

	reply2(req->rdvno, 0, 0, 0);
	return 0;
}
#if 0
int if_exit(fs_request *req)
{
	session_t *session = session_find(unpack_sid(req));
	if (!session)
		return ESRCH;

	session_destroy(session);
	reply2(req->rdvno, 0, 0, 0);
	return 0;
}
#endif
