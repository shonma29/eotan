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
#include <errno.h>
#include <mm.h>
#include <pm.h>
#include <services.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "../../kernel/mpu/interrupt.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "interface.h"
#include "process.h"

static int (*funcs[])(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args) = {
	mm_palloc,
	mm_pfree,
	mm_vmap,
	mm_vunmap,
	mm_clock_gettime,
	mm_process_create,
	mm_process_destroy,
	mm_process_duplicate,
	mm_process_set_context,
	mm_sbrk,
	mm_thread_create
};

#define BUFSIZ (sizeof(mm_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))
#define MYNAME "mm"

static ER init(void);
static ER proxy_initialize(void);
static void proxy(void);
static void doit(void);

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	process_initialize();

	ER result = proxy_initialize();
	if (result)
		return result;

	define_mpu_handlers((FP)default_handler, (FP)stack_fault_handler);

	return kcall->port_open(&pk_cpor);
}

static ER proxy_initialize(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, proxy, pri_server_middle,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	return kcall->thread_create(PORT_PM, &pk_ctsk);
}

static void proxy(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, sizeof(pm_args_t), sizeof(pm_reply_t) };
	ER error = kcall->port_open(&pk_cpor);
	if (error) {
		log_err("proxy: open failed %d\n", error);
		return;
	} else
		log_info("proxy: start\n");

	for (;;) {
		RDVNO rdvno;
		pm_args_t args;
		ER_UINT size = kcall->port_accept(PORT_PM, &rdvno, &args);
		if (size < 0) {
			log_err("proxy: receive failed %d\n", size);
			break;
		}

		args.process_id |= get_rdv_tid(rdvno) << 16;

		pm_reply_t reply;
		size = kcall->port_call(PORT_FS, &args, sizeof(args));
		if (size == sizeof(reply)) {
			pm_reply_t *p = (pm_reply_t*)(&args);
			reply = *p;
		} else {
			log_err("proxy: call failed %d\n", size);
			reply.result1 = -1;
			reply.result2 = 0;
			reply.error_no = ECONNREFUSED;
		}

		int result = kcall->port_reply(rdvno, &reply, sizeof(reply));
		if (result)
			log_err("proxy: reply failed %d\n", result);
	}
}

static void doit(void)
{
	for (;;) {
		mm_args_t args;
		mm_reply_t reply;
		RDVNO rdvno;
		int result;
		ER_UINT size = kcall->port_accept(PORT_MM, &rdvno, &args);

		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			break;
		}

		if (size != sizeof(mm_args_t)) {
			reply.result = -1;
			reply.error_no = EINVAL;
			result = reply_failure;

		} else if (args.syscall_no > NUM_OF_FUNCS) {
			reply.result = -1;
			reply.error_no = ENOTSUP;
			result = reply_failure;

		} else
			result = funcs[args.syscall_no](&reply, rdvno, &args);

		switch (result) {
		case reply_success:
		case reply_failure:
			result = kcall->port_reply(rdvno, &reply,
					sizeof(reply));
			if (result != E_OK)
				log_err(MYNAME ": reply failed %d\n",
						result);
			break;

		default:
			break;
		}
	}
}

void start(VP_INT exinf)
{
	ER error = init();

	if (error)
		log_err(MYNAME ": open failed %d\n", error);

	else {
		log_info(MYNAME ": start\n");
		doit();
		log_info(MYNAME ": end\n");

		error = kcall->port_close();
		if (error)
			log_err(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
