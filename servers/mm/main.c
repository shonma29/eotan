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
#include <init.h>
#include <services.h>
#include <mm/config.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "api.h"
#include "proxy.h"

static int (*funcs[])(mm_request *) = {
	mm_fork,
	mm_exec,
	mm_wait,
	mm_exit,
	mm_vmap,
	mm_vunmap,
	mm_sbrk,
	mm_chdir,
	mm_dup,
	mm_lseek,
	mm_open,
	mm_create,
	mm_read,
	mm_write,
	mm_close,
	mm_remove,
	mm_stat,
	mm_fstat,
	mm_chmod,
	mm_clock_gettime,
	mm_thread_find
};

#define BUFSIZ (sizeof(sys_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))

#define REQUEST_QUEUE_SIZE REQUEST_MAX

static ID receiver_id;
ID worker_id;
static slab_t request_slab;
static volatile lfq_t req_queue;
static char req_buf[
		lfq_buf_size(sizeof(mm_request *), REQUEST_QUEUE_SIZE)
];
static tree_t tag_tree;

static ER initialize(void);
static ER_ID worker_initialize(void);
static void worker(void);
static int worker_enqueue(mm_request **req);
static void doit(void);


static ER initialize(void)
{
	process_initialize();
	file_initialize();
	define_mpu_handlers((FP) default_handler, (FP) page_fault_handler);

	request_slab.unit_size = sizeof(mm_request);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(REQUEST_MAX, PAGE_SIZE,
			sizeof(mm_request));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);
	tree_create(&tag_tree, NULL, NULL);

	lfq_initialize(&req_queue, req_buf, sizeof(mm_request*),
			REQUEST_QUEUE_SIZE);

	receiver_id = kcall->thread_get_id();
	T_CMTX pk_cmtx = {
		TA_CEILING,
		pri_server_high
	};
	ER result = kcall->mutex_create(receiver_id, &pk_cmtx);
	if (result) {
		log_err(MYNAME ": mutex_create failed %d\n", result);
		return result;
	}

	worker_id = worker_initialize();
	if (worker_id < 0) {
		log_err(MYNAME ": worker_initialize failed %d\n", worker_id);
		return worker_id;
	}

//log_info("mm size=%d\n", BUFSIZ);
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	return kcall->ipc_open(&pk_cpor);
}

static ER_ID worker_initialize(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, worker, pri_server_middle,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};
	return kcall->thread_create_auto(&pk_ctsk);
}

static void worker(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	ER_ID result = kcall->ipc_open(&pk_cpor);
	if (result)
		return;

	exec_init(INIT_PID);
	for (;;) {
		mm_request *req;
		if (lfq_dequeue(&req_queue, &req) == QUEUE_OK) {
			switch (funcs[req->args.syscall_no](req)) {
			case reply_success:
			case reply_failure:
				if (req->node.key) {
					sys_reply_t *reply = (sys_reply_t *) &(req->args);
					int result = kcall->ipc_send(req->node.key,
							reply,
							sizeof(*reply));
					if (result)
						log_err(MYNAME ": reply failed %d\n",
								result);
				} else
					//TODO set sequence for inner call
					log_warning(MYNAME ": inner call %d\n",
						req->node.key);
				break;
			default:
				break;
			}

			kcall->mutex_lock(receiver_id, TMO_FEVR);
			if (remove_request(req->node.key, req))
				log_err(MYNAME ": remove tree failed %d\n",
						req->node.key);

			slab_free(&request_slab, req);
			kcall->mutex_unlock(receiver_id);
			kcall->thread_wakeup(receiver_id);
		} else
			kcall->thread_sleep();
	}
}

static int worker_enqueue(mm_request **req)
{
	kcall->mutex_lock(receiver_id, TMO_FEVR);

	if (add_request((*req)->node.key, *req))
		log_err(MYNAME ": add tree failed %d\n", (*req)->node.key);
	else if (lfq_enqueue(&req_queue, req) == QUEUE_OK) {
		kcall->thread_wakeup(worker_id);
		*req = slab_alloc(&request_slab);
		kcall->mutex_unlock(receiver_id);
		return 0;
	}

	kcall->mutex_unlock(receiver_id);
	return ENOMEM;
}

static void doit(void)
{
	for (mm_request *req = slab_alloc(&request_slab);;) {
		if (!req) {
			kcall->thread_sleep();
			kcall->mutex_lock(receiver_id, TMO_FEVR);
			req = slab_alloc(&request_slab);
			kcall->mutex_unlock(receiver_id);
			continue;
		}

		int size = kcall->ipc_receive(PORT_MM, &(req->node.key),
				&(req->args));
		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			continue;
		}

		int result;
		if (size != sizeof(req->args))
			result = EPROTO;
		else if (req->args.syscall_no > NUM_OF_FUNCS)
			result = ENOTSUP;
		else
			result = worker_enqueue(&req);

		if (result) {
			sys_reply_t *reply = (sys_reply_t *) &(req->args);
			reply->result = -1;
			reply->data[0] = result;
			result = kcall->ipc_send(req->node.key, reply,
					sizeof(*reply));
			if (result)
				log_err(MYNAME ": reply failed %d\n", result);
		}
	}
}

void start(VP_INT exinf)
{
	ER error = initialize();
	if (error)
		log_err(MYNAME ": open failed %d\n", error);
	else {
		log_info(MYNAME ": start\n");
		doit();
		log_info(MYNAME ": end\n");

		error = kcall->ipc_close();
		if (error)
			log_err(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}

mm_request *find_request(const int tag)
{
	node_t *node = tree_get(&tag_tree, tag);
	//TODO define getParent macro
	return ((mm_request *) node);
}

int add_request(const int tag, mm_request *req)
{
	//TODO remove this code after test
	if (tree_get(&tag_tree, tag))
		return (-1);

	return (tree_put(&tag_tree, tag, &(req->node)) ? 0 : (-1));
}

int remove_request(const int tag, mm_request *req)
{
	return (tree_remove(&tag_tree, req->node.key) ? 0 : (-1));
}
