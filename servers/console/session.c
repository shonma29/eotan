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
#include <init.h>
#include <limits.h>
#include <fs/config.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "api.h"
#include "console.h"
#include "session.h"

#define PID_NIL (-1)

static session_t session;
static slab_t file_slab;


void if_attach(fs_request *req)
{
	int error_no;
	struct _Tattach *request = &(req->packet.Tattach);
	do {
		if (request->afid != NOFID) {
			error_no = EINVAL;
			break;
		}

		session_t *session = session_create(unpack_sid(req));
		if (!session) {
			//TODO other error when sid exists
			error_no = ENOMEM;
			break;
		}

		int fid = request->fid;
		struct file *file;
		error_no = session_create_file(&file, session, fid);
		if (error_no) {
			session_destroy(session);
			break;
		}

//TODO return error if aname != '/'?

		//TODO walk aname
		//TODO can not walk to the parent of aname
		//TODO bind fid to with session type (for use to close session)
		file->f_flag = O_ACCMODE;
		file->size = 0;
		file->driver = NULL;

		fsmsg_t response;
		response.header.token = req->packet.header.token;
		response.header.type = Rattach;
		response.Rattach.tag = request->tag;
		response.Rattach.qid = session->pid;
		reply(req->tag, &response, MESSAGE_SIZE(Rattach));
log_info("console: attach success %d\n", session->pid);
		return;
	} while (false);
log_info("console: attach failed %d\n", error_no);
	reply_error(req->tag, req->packet.header.token, request->tag,
			error_no);
}

void session_initialize(void)
{
	session.pid = PID_NIL;
	tree_create(&(session.files), NULL, NULL);

	file_slab.unit_size = sizeof(struct file);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = slab_max_block(MAX_TOTAL_FILE, PAGE_SIZE,
			sizeof(struct file));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	slab_create(&file_slab);
}

session_t *session_create(const pid_t pid)
{
	if (session.pid != PID_NIL)
		return NULL;

	session.pid = pid;
	return &session;
}

void session_destroy(session_t *session)
{
//TODO walk tree
	//TODO optimize
	for (int fd = 0; fd < MAX_FILE; fd++)
		if (session_destroy_file(session, fd)) {
			//TODO what to do?
		}

	session->pid = PID_NIL;
}

session_t *session_find(const pid_t pid)
{
	return ((session.pid == pid) ? &session : NULL);
}

int session_create_file(struct file **file, session_t *session, const int fd)
{
	if (session_find_file(session, fd))
		//TODO adequate error code
		return EBUSY;

	struct file *f = slab_alloc(&file_slab);
	if (!f)
		return ENOMEM;

	if (!tree_put(&(session->files), fd, &(f->node))) {
		slab_free(&file_slab, f);
		//TODO adequate error code
		return EIO;
	}

	f->f_flag = 0;
	f->driver = NULL;

	*file = f;
	return 0;
}

int session_destroy_file(session_t *session, const int fd)
{
	struct file *file = (struct file *) tree_remove(&(session->files), fd);
	if (!file)
		return EBADF;

	slab_free(&file_slab, file);
	return 0;
}

struct file *session_find_file(const session_t *session, const int fd)
{
	return (struct file *) tree_get(&(session->files), fd);
}
