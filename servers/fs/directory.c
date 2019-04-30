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


int if_create(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	vnode_t *starting_node;
	int error_no = session_get_path(req->buf, &starting_node,
			session, unpack_tid(req), (char*)(req->packet.arg1));
	if (error_no)
		return error_no;

	vnode_t *node;
	error_no = vfs_create(starting_node, req->buf, O_RDONLY,
			req->packet.arg2, &(session->permission), &node);
	if (error_no)
		return error_no;

	vnodes_remove(node);
	reply2(req->rdvno, 0, 0, 0);

	return 0;
}

int if_remove(fs_request *req)
{
	session_t *session = session_find(unpack_pid(req));
	if (!session)
		return ESRCH;

	vnode_t *starting_node;
	int error_no = session_get_path(req->buf, &starting_node,
			session, unpack_tid(req), (char*)(req->packet.arg1));
	if (error_no)
		return error_no;

	error_no = vfs_remove(starting_node, req->buf, &(session->permission));
	if (error_no)
		return error_no;

	reply2(req->rdvno, 0, 0, 0);

	return 0;
}
