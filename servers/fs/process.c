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
#include <boot/init.h>
#include <nerve/kcall.h>
#include "api.h"
#include "session.h"
#include "procfs/process.h"
#include "../../lib/libserv/libmm.h"


int if_fork(fs_request *req)
{
	session_t *parent = session_find(unpack_pid(req));
	if (!parent)
		return ESRCH;

	pid_t pid;
	for (pid = INIT_PID + 1; pid < MAX_SESSION; pid++) {
		if (!session_find(pid))
			break;
	}
	if (pid >= MAX_SESSION)
		return ENOMEM;

	if (process_duplicate(parent->node.key, pid) == -1)
		return ENOMEM;

	session_t *child = session_create(pid);
	if (!child) {
		//TODO destroy process
		return ENOMEM;
	}

	proc_duplicate(parent, child);

	ID thread_id = thread_create(pid, (FP)(req->packet.arg2),
			(VP)(req->packet.arg1));
	if (thread_id < 0) {
		//TODO destroy process
		session_destroy(child);
		//TODO adequate errno
		return ENOMEM;
	}

	if (kcall->thread_start(thread_id) < 0) {
		//TODO destroy process
		session_destroy(child);
		//TODO adequate errno
		return ENOMEM;
	}

	reply2(req->rdvno, 0, pid, 0);
	return 0;
}

int if_exec(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
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

int if_exit(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	session_destroy(session);
	reply2(req->rdvno, 0, 0, 0);
	return 0;
}
