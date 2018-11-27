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
#include <core.h>
#include <fcntl.h>
#include <limits.h>
#include <boot/init.h>
#include <core/options.h>
#include <fs/vfs.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "api.h"
#include "procfs/process.h"


int if_chdir(fs_request *req)
{
	session_t *session = session_find(req->packet.process_id);
	if (!session)
		return ESRCH;

	vnode_t *starting_node;
	int error_no = session_get_path(&starting_node, req->packet.process_id,
			get_rdv_tid(req->rdvno), (UB*)(req->packet.arg1),
			(UB*)(req->buf));
	if (error_no)
		return error_no;

	vnode_t *wd;
	error_no = vfs_walk(starting_node, req->buf, O_RDONLY,
			&(session->permission), &wd);
	if (error_no)
		return error_no;

	if ((wd->mode & S_IFMT) != S_IFDIR) {
		vnodes_remove(wd);
		return ENOTDIR;
	}

	//TODO not need in plan 9
	error_no = vfs_permit(wd, &(session->permission), X_OK);
	if (error_no) {
		vnodes_remove(wd);
		return error_no;
	}

	vnodes_remove(session->cwd);
	session->cwd = wd;

	reply2(req->rdvno, 0, 0, 0);

	return 0;
}

session_t *session_find(const pid_t pid)
{
	if ((pid < INIT_PID)
			|| (pid >= MAX_PROCESS))
		return NULL;

	return &(proc_table[pid].session);
}

int session_get_path(vnode_t **ip, const ID pid, const ID tid,
	UB *src, UB *dest)
{
    ER_UINT len = kcall->region_copy(tid, src, PATH_MAX + 1, dest);
    if (len <= 0)
	return EINVAL;

    if (len > PATH_MAX)
	return ENAMETOOLONG;

    if (*dest == '/') {
	*ip = rootfile;

	return 0;
    }

    return proc_get_cwd(pid, ip);
}
