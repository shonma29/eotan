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
#include <boot/init.h>
#include <fs/config.h>
#include <mpu/memory.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <sys/errno.h>
#include "fs.h"
#include "devfs/devfs.h"
#include "procfs/process.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"

#define REQUEST_QUEUE_SIZE (32)

static ID receiver_id;
static ID worker_id;
static slab_t request_slab;
static volatile lfq_t req_queue;
static char req_buf[
		lfq_buf_size(sizeof(fs_request *), REQUEST_QUEUE_SIZE)
];
static int (*syscall[])(fs_request*) = {
	if_fork,
	if_waitpid,
	if_exec,
	if_exit,
	if_kill,
	if_chdir,
	if_create,
	if_remove,
	if_fstat,
	if_chmod,
	if_dup2,
	if_open,
	if_lseek,
	if_read,
	if_write,
	if_close
};

static int initialize(void);
static bool port_init(void);
static void request_init(void);
static ER_UINT worker_init(void);
static void work(void);


static int initialize(void)
{
	session_initialize();

	if (fs_initialize())
		return -1;

	T_CMTX pk_cmtx = {
		TA_CEILING,
		pri_server_high
	};
	ER result;

	receiver_id = kcall->thread_get_id();
	result = kcall->mutex_create(receiver_id, &pk_cmtx);
	if (result) {
		log_err("fs: mutex_create failed %d\n", result);
		return -1;
	}

	if (!port_init()) {
		log_err("fs: port_init failed\n");
		return -1;
	}

	if (!device_init())
		return -1;

	request_init();
	worker_id = worker_init();
	if (worker_id < 0) {
		log_err("fs: worker_init failed %d\n", worker_id);
		return -1;
	}

	init_process();

	if (device_find(sysinfo->root.device)
			&& device_find(get_device_id(DEVICE_MAJOR_CONS, 0))) {
		if (fs_mount(sysinfo->root.device)) {
			log_err("fs: fs_mount(%x, %d) failed\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
		} else {
			log_info("fs: fs_mount(%x, %d) succeeded\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
			exec_init(INIT_PID, INIT_PATH_NAME);
		}
	}

	log_info("fs: start\n");

	return 0;
}

static bool port_init(void)
{
	struct t_cpor packet = {
		TA_TFIFO,
		sizeof(pm_args_t),
		sizeof(pm_reply_t)
	};

	return kcall->port_open(&packet) == E_OK;
}

static void request_init(void)
{
	request_slab.unit_size = sizeof(fs_request);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = MAX_REQUEST
			/ ((PAGE_SIZE - sizeof(slab_block_t))
					/ sizeof(fs_request));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);

	lfq_initialize(&req_queue, req_buf, sizeof(fs_request*),
			REQUEST_QUEUE_SIZE);
}

static ER_UINT worker_init(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT)NULL,
		(FP)work,
		pri_server_middle,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};

	return kcall->thread_create_auto(&pk_ctsk);
}

static void work(void)
{
	for (;;) {
		fs_request *req;

		if (lfq_dequeue(&req_queue, &req) == QUEUE_OK) {
			int result = syscall[req->packet.operation](req);

			if (result > 0)
				reply2(req->rdvno, result, -1, 0);

			kcall->mutex_lock(receiver_id, TMO_FEVR);
			slab_free(&request_slab, req);
			kcall->mutex_unlock(receiver_id);
			kcall->thread_wakeup(receiver_id);

		} else
			kcall->thread_sleep();
	}
}

void start(VP_INT exinf)
{
	fs_request *req;

	if (initialize()) {
		kcall->thread_end_and_destroy();
		return;
	}

	for (req = slab_alloc(&request_slab);;) {
		ER_UINT size;

		if (!req) {
			kcall->thread_sleep();
			kcall->mutex_lock(receiver_id, TMO_FEVR);
			req = slab_alloc(&request_slab);
			kcall->mutex_unlock(receiver_id);
			continue;
		}

		size = kcall->port_accept(PORT_FS, &(req->rdvno),
				&(req->packet));
		if (size == sizeof(pm_args_t)) {
			int result;

			if ((int)(req->packet.operation) >=
					sizeof(syscall) / sizeof(syscall[0]))
				result = ENOTSUP;

			else if (lfq_enqueue(&req_queue, &req) == QUEUE_OK) {
				kcall->thread_wakeup(worker_id);
				kcall->mutex_lock(receiver_id, TMO_FEVR);
				req = slab_alloc(&request_slab);
				kcall->mutex_unlock(receiver_id);
				continue;

			} else
				result = ENOMEM;

			reply2(req->rdvno, result, -1, 0);
		}

		else if (size < 0)
			log_err("fs: receive failed %d\n", size);

		else
			reply2(req->rdvno, EINVAL, -1, 0);
	}
}
