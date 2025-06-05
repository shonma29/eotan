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
#include <fcntl.h>
#include <fs/config.h>
#include <nerve/kcall.h>
#include "hmi.h"
#include "session.h"

#define BAR_HEIGHT (20)
#define DEFAULT_WIDTH (1024 / 2)
#define DEFAULT_HEIGHT ((768 - BAR_HEIGHT) / 2)

enum {
	SESSION_SLAB = 0x01,
	FILE_SLAB = 0x02
};

static tree_t sessions;
static slab_t session_slab;
static slab_t file_slab;
list_t session_list;
session_t *focused_session = NULL;
static int initialized_resources = 0;
static int num_of_window = 0;

static session_t *_create(const int);
static void _destroy(session_t *);


int if_attach(fs_request_t *req)
{
	int error_no;
	struct _Tattach *request = &(req->packet.Tattach);
	do {
		if (request->afid != NOFID) {
			error_no = EINVAL;
			break;
		}

		session_t *session = _create(unpack_sid(req));
		if (!session) {
			error_no = ENOMEM;
			break;
		}

		session->tid = unpack_tid(req);

		struct file *file;
		error_no = session_create_file(&file, session, request->fid);
		if (error_no) {
			_destroy(session);
			break;
		}

		//TODO return error if aname != '/'?
		//TODO walk aname
		//TODO can not walk to the parent of aname
		//TODO bind fid to with session type (for use to close session)
		file->f_flag = O_ACCMODE;

		window_t *w;
		//TODO !define function to calculate position
		error_no = window_create(&w, 0,
				BAR_HEIGHT + DEFAULT_HEIGHT * num_of_window,
				DEFAULT_WIDTH,
				BAR_HEIGHT + DEFAULT_HEIGHT * (num_of_window + 1),
				WINDOW_ATTR_HAS_BORDER | WINDOW_ATTR_HAS_TITLE);
		if (error_no) {
			session_destroy_file(session, file->node.key);
			break;
		}

		session->window = w;
		num_of_window++;
		list_insert(&session_list, &(session->brothers));
		window_focus(-1);

		fsmsg_t *response = &(req->packet);
		//response->header.token = req->packet.header.token;
		response->header.type = Rattach;
		//response->Rattach.tag = request->tag;
		response->Rattach.qid = session->node.key;
		reply(req, MESSAGE_SIZE(Rattach));
		return 0;
	} while (false);

	return error_no;
}

int session_initialize(void)
{
	tree_create(&sessions, NULL, NULL);
	list_initialize(&session_list);

	session_slab.unit_size = sizeof(session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = slab_max_block(MAX_SESSION, PAGE_SIZE,
			sizeof(session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;

	if (slab_create(&session_slab))
		return SESSION_SLAB;
	else
		initialized_resources |= SESSION_SLAB;

	file_slab.unit_size = sizeof(struct file);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = slab_max_block(MAX_FILE, PAGE_SIZE,
			sizeof(struct file));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;

	if (slab_create(&file_slab))
		return FILE_SLAB;
	else
		initialized_resources |= FILE_SLAB;

	return 0;
}

static session_t *_create(const int sid)
{
	session_t *session = slab_alloc(&session_slab);
	if (!session)
		return NULL;

	session->event.buf = kcall->palloc();
	if (!(session->event.buf)) {
		slab_free(&session_slab, session);
		return NULL;
	}

	tree_create(&(session->files), NULL, NULL);

	if (!tree_put(&sessions, sid, &(session->node))) {
		slab_free(&session_slab, session);
		//TODO return EADDRINUSE
		return NULL;
	}

	list_initialize(&(session->brothers));
	session->window = NULL;
	list_initialize(&(session->event.readers));
	// buffer size must be power of 2
	session->event.size = PAGE_SIZE;
	session->event.read_position = 0;
	session->event.write_position = 0;
	session->event.position_mask = session->event.size - 1;
	return session;
}

static void _destroy(session_t *session)
{
	if (!tree_remove(&sessions, session->node.key))
		return;

	list_remove(&(session->brothers));
	window_focus(-1);

	if (session->window) {
		window_destroy(session->window);
		num_of_window--;
	}

	//TODO !event.readers should be released
	kcall->pfree(session->event.buf);
	slab_free(&session_slab, session);
}
#if 0
static void _release_requests(session_t *p)
{
	for (list_t *head = list_next(&(p->event.readers));
			!list_is_edge(&(p->event.readers), head);) {
		fs_request_t *req = (fs_request_t *) getRequestFromList(head);
		head = list_next(head);
		reply_read(req);
		dequeue_request(req);
	}
}
#endif
session_t *session_find_by_request(const fs_request_t *req)
{
	node_t *node = tree_get(&sessions, unpack_sid(req));
	return (node ? ((session_t *) getSessionPtr(node)) : NULL);
}

int session_create_file(struct file **file, session_t *session, const int fid)
{
	struct file *f = slab_alloc(&file_slab);
	if (!f)
		return ENFILE;

	if (!tree_put(&(session->files), fid, &(f->node))) {
		slab_free(&file_slab, f);
		return EADDRINUSE;
	}

	list_initialize(&(f->requests));
	f->f_session = session;
	f->f_flag = 0;
	f->f_driver = NULL;
	*file = f;
	return 0;
}

int session_destroy_file(session_t *session, const int fid)
{
	struct file *file = (struct file *) tree_remove(&(session->files), fid);
	if (!file)
		return EBADF;

	//TODO !requests should be released
	slab_free(&file_slab, file);

	if (!tree_size(&(session->files)))
		_destroy(session);

	return 0;
}

void session_destroy_all_files(void)
{
	for (node_t *s; (s = tree_root(&sessions));) {
		session_t *session = (session_t *) getSessionPtr(s);
		for (node_t *n; (n = tree_root(&(session->files)));)
			session_destroy_file(session, n->key);
	}

	slab_destroy(&file_slab);
	slab_destroy(&session_slab);
}

struct file *session_find_file(const session_t *session, const int fid)
{
	return (struct file *) tree_get(&(session->files), fid);
}
