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
#include <fs/config.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "pipe.h"
#include "api.h"
#include "session.h"

#define REQUEST_MAX (16)

struct fs_func {
	int (*call)(fs_request_t *);
	size_t max;
};

static int no_support(fs_request_t *);

static struct fs_func func_table[] = {
	{ if_attach, MESSAGE_SIZE(Tattach) },
	{ if_walk, MESSAGE_SIZE(Twalk) },
	{ if_open, MESSAGE_SIZE(Topen) },
	{ no_support, MESSAGE_SIZE(Tcreate) },
	{ if_read, MESSAGE_SIZE(Tread) },
	{ if_write, MESSAGE_SIZE(Twrite) },
	{ if_clunk, MESSAGE_SIZE(Tclunk) },
	{ no_support, MESSAGE_SIZE(Tremove) },
	{ no_support, MESSAGE_SIZE(Tstat) },
	{ no_support, MESSAGE_SIZE(Twstat) }
};
#define NUM_OF_FUNC (sizeof(func_table) / sizeof(func_table[0]))

static slab_t request_slab;
static fs_request_t req;

static int initialize(void);
static int accept(void);


fs_request_t *enqueue_request(list_t *queue, const fs_request_t *req)
{
	fs_request_t *r = slab_alloc(&request_slab);
	if (r) {
		r->position = 0;
		r->packet = req->packet;
		r->tag = req->tag;
		r->tid = thread_id_of_token(req->packet.header.token);
		list_enqueue(queue, &(r->brothers));
	}

	return r;
}

void dequeue_request(fs_request_t *req)
{
	list_remove(&(req->brothers));
	slab_free(&request_slab, req);
}

static int no_support(fs_request_t *req)
{
	return ENOTSUP;
}

static int initialize(void)
{
	struct t_cpor pk_cpor = {
		TA_TFIFO,
		sizeof(fsmsg_t),
		sizeof(fsmsg_t)
	};
	int result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": failed to open %d\n", result);
		return (-1);
	}

	session_initialize();

	request_slab.unit_size = sizeof(fs_request_t);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(MAX_FILE, PAGE_SIZE,
			sizeof(fs_request_t));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);
	return 0;
}

static int accept(void)
{
	do {
		ER_UINT size = kcall->ipc_receive(MYPORT, &(req.tag),
				&(req.packet));
		if (size < 0) {
			log_warning(MYNAME ": failed to receive %d\n", size);
			break;
		}

		int result;
		if ((size < sizeof(req.packet.header))
				|| (req.packet.header.ident != IDENT)) {
			result = reply_error(req.tag, 0, 0, EPROTO);
			if (result)
				log_warning(MYNAME ": failed to reply %d\n",
						result);

			break;
		}

		if (req.packet.header.type >= NUM_OF_FUNC) {
			result = ENOTSUP;
		} else if (size == func_table[req.packet.header.type].max) {
			result = func_table[req.packet.header.type].call(&req);
		} else
			result = EPROTO;

		if (result) {
			result = reply_error(req.tag, req.packet.header.token,
					req.packet.Rerror.tag, result);
			if (result)
				log_warning(MYNAME ": failed to reply %d\n",
						result);
		}
	} while (false);

	return 0;
}

void start(VP_INT exinf)
{
	if (!initialize()) {
		log_info(MYNAME ": start\n");
		while (!accept());
		log_info(MYNAME ": end\n");

		int result = kcall->ipc_close();
		if (result)
			log_err(MYNAME ": failed to close %d\n", result);

		session_destroy_all_files();
		slab_destroy(&request_slab);
	}

	kcall->thread_end_and_destroy();
}
