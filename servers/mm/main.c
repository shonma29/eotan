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
#include <string.h>
#include <mpufunc.h>
#include <mm/config.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include "mm.h"
#include "api.h"
#include "proxy.h"
#include "device.h"

enum {
	PROCESSES = 0x01,
	FILES = 0x02,
	DEVICES = 0x04,
	REQUEST_SLAB = 0x08,
	PORT = 0x10,
	HANDLERS = 0x20,
	INIT = 0x40
};

static int (*funcs[])(mm_request_t *) = {
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
	mm_pipe,
	mm_bind,
	mm_unmount,
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
	mm_kill
};

#define BUFSIZ (sizeof(sys_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))

static slab_t request_slab;
static tree_t tag_tree;
static void *receiver_sp;
static void *fiber_stacks_head = NULL;
static size_t fiber_stacks_count = 0;
static int initialized_resources = 0;

static int initialize(void);
static void execute(mm_request_t *);
static void accept(void);
static mm_request_t *find_request(const int);
static int add_request(const int, mm_request_t *);
static int remove_request(const int, mm_request_t *);
static void *getFiberStack(void);
static void giveBackFiberStack(void *);


static int initialize(void)
{
	if (process_initialize())
		return PROCESSES;
	else
		initialized_resources |= PROCESSES;

	if (file_initialize())
		return FILES;
	else
		initialized_resources |= FILES;

	if (device_initialize())
		return DEVICES;
	else
		initialized_resources |= DEVICES;

	request_slab.unit_size = sizeof(mm_request_t);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(REQUEST_MAX, PAGE_SIZE,
			sizeof(mm_request_t));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	if (slab_create(&request_slab))
		return REQUEST_SLAB;
	else
		initialized_resources |= REQUEST_SLAB;

	tree_create(&tag_tree, NULL, NULL);

	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	int result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": open failed %d\n", result);
		return PORT;
	} else
		initialized_resources |= PORT;

	if (define_mpu_handlers(default_handler, page_fault_handler))
		return HANDLERS;
	else
		initialized_resources |= HANDLERS;

	result = create_init(INIT_PID, init);
	if (result) {
		log_err(MYNAME ": init failed %d\n", result);
		return INIT;
	} else
		initialized_resources |= INIT;

	return 0;
}

static void execute(mm_request_t *req)
{
	if (add_request(req->node.key, req)) {
		log_err(MYNAME ": add tree failed %d\n", req->node.key);
		return;
	}

	int func_result = funcs[req->args.syscall_no](req);
	switch (func_result) {
	case reply_success:
	case reply_failure: {
			sys_reply_t *reply = (sys_reply_t *) &(req->args);
			int result = kcall->ipc_send(req->node.key,
					reply, sizeof(*reply));
			if (result
					&& (func_result != reply_no_caller))
				log_err(MYNAME ": reply failed %d\n", result);
		}
		break;
	default:
		break;
	}

	if (remove_request(req->node.key, req))
		log_err(MYNAME ": remove tree failed %d\n", req->node.key);
}

static void accept(void)
{
	int rdvno;
	sys_args_t args;
	int size = kcall->ipc_receive(PORT_MM, &rdvno, &args);
	if (size < sizeof(args.syscall_no)) {
		log_err(MYNAME ": receive failed %d\n", size);
		return;
	}

	fsmsg_t *message = (fsmsg_t *) &args;
	if (message->header.ident == IDENT) {
		do {
			if (size < sizeof(message->header)
					+ sizeof(message->Rerror.tag)) {
				//TODO debug
				log_warning("9p: bad size %d\n", size);
				break;
			}

			mm_request_t *req = find_request(message->Rerror.tag);
			if (!req) {
				//TODO debug
				log_warning("9p: cannot find %d\n",
						message->Rerror.tag);
				if (message->header.type == Rerror)
					log_warning("9p: error %d\n",
							message->Rerror.ename);
				break;
			}

			//TODO check callee
			req->size = size;
			memcpy(&(req->message), message, size);
			if (fiber_switch(&receiver_sp, &(req->fiber_sp))) {
				giveBackFiberStack(req->stack);
				slab_free(&request_slab, req);
			}
		} while (false);

		return;
	}

	int result;
	do {
		if (size != sizeof(args)) {
			result = EPROTO;
			break;
		}

		if (args.syscall_no > NUM_OF_FUNCS) {
			result = ENOTSUP;
			break;
		}

		mm_request_t *req = slab_alloc(&request_slab);
		if (!req) {
			result = ENOMEM;
			break;
		}

		req->stack = getFiberStack();
		if (!(req->stack)) {
			slab_free(&request_slab, req);
			result = E_NOMEM;
			break;
		}

		req->node.key = rdvno;
		//TODO optimize
		memcpy(&(req->args), &args, sizeof(args));

		req->receiver_sp = &receiver_sp;
		if (fiber_start((void *) (((uintptr_t) req->stack) + PAGE_SIZE),
				execute, req, &receiver_sp, &(req->fiber_sp))) {
			giveBackFiberStack(req->stack);
			slab_free(&request_slab, req);
		}

		return;
	} while (false);

	sys_reply_t *reply = (sys_reply_t *) &args;
	reply->result = -1;
	reply->data[0] = result;
	result = kcall->ipc_send(rdvno, reply, sizeof(*reply));
	if (result)
		log_err(MYNAME ": reply failed %d\n", result);
}

void start(VP_INT exinf)
{
	int error = initialize();
	if (error)
		log_err(MYNAME ": initialize failed %d\n", error);
	else {
		log_info(MYNAME ": start\n");

		for (;;)
			accept();

		log_info(MYNAME ": end\n");
		error = kcall->ipc_close();
		if (error)
			log_err(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}

mm_request_t *find_request(const int tag)
{
	node_t *node = tree_get(&tag_tree, tag);
	return (node ? (mm_request_t *) getParent(mm_request_t, node) : NULL);
}

int add_request(const int tag, mm_request_t *req)
{
	return (tree_put(&tag_tree, tag, &(req->node)) ? 0 : (-1));
}

int remove_request(const int tag, mm_request_t *req)
{
	return (tree_remove(&tag_tree, req->node.key) ? 0 : (-1));
}

static void *getFiberStack(void)
{
	void *p;

	if (fiber_stacks_count) {
		p = fiber_stacks_head;
		fiber_stacks_head = (void *) (*((uintptr_t *) p));
		fiber_stacks_count--;
	} else
		p = kcall->palloc();

	return p;
}

static void giveBackFiberStack(void *p)
{
	if (fiber_stacks_count >= FIBER_POOL_MAX) {
		kcall->pfree(p);
	} else {
		uintptr_t *block = (uintptr_t *) p;
		*block = (uintptr_t) fiber_stacks_head;
		fiber_stacks_head = block;
		fiber_stacks_count++;
	}
}
