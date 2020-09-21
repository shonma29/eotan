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
#include <mm/config.h>
#include <nerve/kcall.h>
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
	mm_attach,
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

static slab_t request_slab;
static tree_t tag_tree;
ID receiver_id;

static int initialize(void);
static void execute(mm_request *);
static void accept(void);


static int initialize(void)
{
	process_initialize();
	file_initialize();
	define_mpu_handlers(default_handler, page_fault_handler);

	request_slab.unit_size = sizeof(mm_request);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = slab_max_block(REQUEST_MAX, PAGE_SIZE,
			sizeof(mm_request));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);
	tree_create(&tag_tree, NULL, NULL);

	receiver_id = kcall->thread_get_id();

	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	int result = kcall->ipc_open(&pk_cpor);
	if (result)
		return result;

	create_init(INIT_PID, init);
	return 0;
}

static void execute(mm_request *req)
{
	if (add_request(req->node.key, req)) {
		log_err(MYNAME ": add tree failed %d\n", req->node.key);
		return;
	}

	switch (funcs[req->args.syscall_no](req)) {
	case reply_success:
	case reply_failure: {
			sys_reply_t *reply = (sys_reply_t *) &(req->args);
			int result = kcall->ipc_send(req->node.key,
					reply, sizeof(*reply));
			if (result)
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
	if (size < 0) {
		log_err(MYNAME ": receive failed %d\n", size);
		return;
	}

	int result;
	if (size != sizeof(args))
		result = EPROTO;
	else if (args.syscall_no > NUM_OF_FUNCS)
		result = ENOTSUP;
	else {
		mm_request *req = slab_alloc(&request_slab);
		if (req) {
			req->node.key = rdvno;
			memcpy(&(req->args), &args, sizeof(args));
			execute(req);
			slab_free(&request_slab, req);
			return;
		} else
			result = ENOMEM;
	}

	sys_reply_t *reply = (sys_reply_t *) &args;
	reply->result = -1;
	reply->data[0] = result;
	result = kcall->ipc_send(rdvno, reply, sizeof(*reply));
	if (result)
		log_err(MYNAME ": reply failed %d\n", result);
}

void start(VP_INT exinf)
{
	ER error = initialize();
	if (error)
		log_err(MYNAME ": open failed %d\n", error);
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
