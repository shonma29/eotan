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
#include <limits.h>
#include <core/options.h>
#include <fs/config.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "fs.h"
#include "session.h"

#define MIN_AUTO_FD (3)

static slab_t session_slab;
static tree_t session_tree;
static slab_t file_slab;


int if_chdir(fs_request *req)
{
	session_t *session = session_find(req->packet.process_id);
	if (!session)
		return ESRCH;

	vnode_t *starting_node;
	int error_no = session_get_path((UB*)(req->buf), &starting_node,
			session, get_rdv_tid(req->rdvno),
			(UB*)(req->packet.arg1));
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

void session_initialize(void)
{
	session_slab.unit_size = sizeof(session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = MAX_SESSION
			/ ((PAGE_SIZE - sizeof(slab_block_t))
					/ sizeof(session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;
	slab_create(&session_slab);
	tree_create(&session_tree, NULL);

	file_slab.unit_size = sizeof(struct file);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = MAX_TOTAL_FILE
			/ ((PAGE_SIZE - sizeof(slab_block_t))
					/ sizeof(struct file));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	slab_create(&file_slab);
}

session_t *session_create(const pid_t pid)
{
	//TODO error when exists?
	session_t *session = (session_t*)tree_get(&session_tree, pid);

	if (!session) {
		session = slab_alloc(&session_slab);
		if (!session)
			return NULL;
	}

	if (!tree_put(&session_tree, pid, &(session->node))) {
		slab_free(&session_slab, session);
		return NULL;
	}

	//TODO adhoc. clear other fields in here?
	session->session_id = pid;
	tree_create(&(session->files), NULL);

	return session;
}

void session_destroy(session_t *session)
{
	//TODO what to do?
	if (!tree_remove(&session_tree, session->node.key))
		return;

	//TODO optimize
	for (int fd = 0; fd < MAX_FILE; fd++)
		if (session_destroy_desc(session, fd)) {
			//TODO what to do?
		}

	slab_free(&session_slab, session);
}

session_t *session_find(const pid_t pid)
{
	return (session_t*)tree_get(&session_tree, pid);
}

int session_create_desc(struct file **file, session_t *session, const int fd)
{
	int d;

	if (fd >= 0) {
		if (session_find_desc(session, fd))
			//TODO adequate error code
			return EBUSY;

		d = fd;
	} else {
		//TODO optimize
		for (d = MIN_AUTO_FD; d < MAX_FILE; d++) {
			if (!session_find_desc(session, d))
				break;
		}
		if (d >= MAX_FILE)
			return EMFILE;
	}

	struct file *f = slab_alloc(&file_slab);
	if (!f)
		return ENOMEM;

	if (!tree_put(&(session->files), d, &(f->node))) {
		slab_free(&file_slab, f);
		//TODO adequate error code
		return EIO;
	}

	f->f_vnode = NULL;
	f->f_flag = 0;
	f->f_count = 0;
	f->f_offset = 0;

	*file = f;
	return 0;
}

int session_destroy_desc(session_t *session, const int fd)
{
	struct file *file = (struct file*)tree_remove(&(session->files), fd);
	if (!file)
		return EBADF;

	int error_no = vnodes_remove(file->f_vnode);

	slab_free(&file_slab, file);
	return error_no;
}

//TODO session should to be const
struct file *session_find_desc(session_t *session, const int fd)
{
	return (struct file*)tree_get(&(session->files), fd);
}

int session_get_path(unsigned char *dest, vnode_t **vnode,
	const session_t *session, const int tid, unsigned char *src)
{
	ER_UINT len = kcall->region_copy(tid, src, PATH_MAX + 1, dest);
	if (len <= 0)
		return EFAULT;

	if (len > PATH_MAX)
		return ENAMETOOLONG;

	if (*dest == '/') {
		*vnode = rootfile;
		return 0;
	}

	*vnode = session->cwd;
	return 0;
}
