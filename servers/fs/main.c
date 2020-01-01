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
#include <major.h>
#include <services.h>
#include <dev/units.h>
#include <fs/config.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <sys/errno.h>
#include "api.h"
#include "fs.h"
#include "session.h"
#include "../../lib/libserv/libserv.h"

struct fs_func {
	void (*call)(fs_request *);
	size_t max;
};

static ID receiver_id;
static ID worker_id;
static slab_t request_slab;
static volatile lfq_t req_queue;
static char req_buf[lfq_buf_size(sizeof(fs_request *), MAX_REQUEST)];
static srv_unit_t unit;
static struct fs_func func_table[] = {
	{ if_attach, MESSAGE_SIZE(Tattach) },
	{ if_walk, MESSAGE_SIZE(Twalk) },
	{ if_open, MESSAGE_SIZE(Topen) },
	{ if_create, MESSAGE_SIZE(Tcreate) },
	{ if_read, MESSAGE_SIZE(Tread) },
	{ if_write, MESSAGE_SIZE(Twrite) },
	{ if_clunk, MESSAGE_SIZE(Tclunk) },
	{ if_remove, MESSAGE_SIZE(Tremove) },
	{ if_stat, MESSAGE_SIZE(Tstat) },
	{ if_wstat, MESSAGE_SIZE(Twstat) }
};
#define NUM_OF_FUNC (sizeof(func_table) / sizeof(func_table[0]))

static int initialize(void);
static ER_ID worker_initialize(void);
static void worker(void);


static int initialize(void)
{
	session_initialize();

	int result = fs_initialize((int) (sysinfo->root.device),
			sysinfo->root.block_size);
	if (result) {
		log_err(MYNAME ": fs_initialize(%s, %d) failed %d\n",
				sysinfo->root.device, sysinfo->root.fstype,
				result);
		return -1;
	}

	log_info(MYNAME ": fs_mount(%s, %d)\n",
			sysinfo->root.device, sysinfo->root.fstype);

	receiver_id = kcall->thread_get_id();
	T_CMTX pk_cmtx = {
		TA_CEILING,
		pri_server_high
	};
	result = kcall->mutex_create(receiver_id, &pk_cmtx);
	if (result) {
		log_err(MYNAME ": mutex_create failed %d\n", result);
		return -1;
	}

	worker_id = worker_initialize();
	if (worker_id < 0) {
		log_err(MYNAME ": worker_initialize failed %d\n", worker_id);
		return -1;
	}

	struct t_cpor pk_cpor = {
		TA_TFIFO,
		sizeof(fsmsg_t),
		sizeof(fsmsg_t)
	};
	result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": ipc_open failed %d\n", result);
		return -1;
	}

	return 0;
}

static ER_ID worker_initialize(void)
{
	request_slab.unit_size = sizeof(fs_request);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(MAX_REQUEST, PAGE_SIZE,
			sizeof(fs_request));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);

	lfq_initialize(&req_queue, req_buf, sizeof(fs_request *), MAX_REQUEST);

	const char *server = DEVICE_CONTROLLER_SERVER;
	device_info_t *info = device_find(server);
	if (!info) {
		log_err(MYNAME ": not found %s\n", server);
		return (-1);
	}

	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT) NULL,
		(FP) worker,
		pri_server_middle,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
	int thread_id = kcall->thread_create_auto(&pk_ctsk);
	if (thread_id >= 0) {
		unit.name = MYNAME;
		unit.queue = &req_queue;
		unit.thread_id = thread_id;
		if (info->driver->create(&unit)) {
			log_err(MYNAME ": cannot create in %s\n", server);
			kcall->thread_destroy(worker_id);
			return (-1);
		} else
			log_info(MYNAME ": created in %s\n", server);
	}

	return thread_id;
}

static void worker(void)
{
	for (;;) {
		fs_request *req;
		if (lfq_dequeue(&req_queue, &req) == QUEUE_OK) {
			func_table[req->packet.header.type].call(req);
			kcall->mutex_lock(receiver_id, TMO_FEVR);
			slab_free(&request_slab, req);
			kcall->mutex_unlock(receiver_id);
			kcall->thread_wakeup(receiver_id);
		} else
			kcall->thread_sleep();
	}
}

static int worker_enqueue(fs_request **req)
{
	 if (lfq_enqueue(&req_queue, req) == QUEUE_OK) {
		kcall->thread_wakeup(worker_id);
		kcall->mutex_lock(receiver_id, TMO_FEVR);
		*req = slab_alloc(&request_slab);
		kcall->mutex_unlock(receiver_id);
		return 0;
	}

	return ENOMEM;
}

void start(VP_INT exinf)
{
	if (initialize()) {
		kcall->thread_end_and_destroy();
		return;
	}

	log_info(MYNAME ": start\n");

	for (fs_request *req = slab_alloc(&request_slab);;) {
		if (!req) {
			kcall->thread_sleep();
			kcall->mutex_lock(receiver_id, TMO_FEVR);
			req = slab_alloc(&request_slab);
			kcall->mutex_unlock(receiver_id);
			continue;
		}

		ER_UINT size = kcall->ipc_receive(PORT_FS, &(req->tag),
				&(req->packet));
		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			continue;
		}

		if (size < sizeof(req->packet.header)) {
			//TODO what is tag?
			reply_error(req->tag, 0, 0, EPROTO);
			continue;
		}
/*
		//TODO validate session
		pid_t pid = thread_find(unpack_tid(req));
		if (pid == -1) {
			log_err(MYNAME ": find failed %d\n", pid);
			//TODO what to do?
			reply2(req->tag, EINVAL, -1, 0);
			continue;
		}
*/

		int result;
		if (req->packet.header.type >= NUM_OF_FUNC)
			result = ENOTSUP;
		else {
			if (size == func_table[req->packet.header.type].max)
				result = worker_enqueue(&req);
			else
				result = EPROTO;
		}

		if (result)
			//TODO what is tag?
			reply_error(req->tag, req->packet.header.token, 0,
					result);
	}
}
