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
#include <kthread.h>
#include <mpu/io.h>
#include <services.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include <set/lf_queue.h>
#include <sys/syscall.h>
#include "delay.h"
#include "func.h"
#include "mpu/mpufunc.h"
#include "arch/archfunc.h"

#define KQUEUE_SIZE 1024

volatile int delay_start = FALSE;
static char kqbuf[lfq_buf_size(sizeof(delay_param_t), KQUEUE_SIZE)];

static ER_ID attach(void);
static void process(VP_INT exinf);
static void detach(void);
static ER kill(const int pid);
static void raise(list_t *w);

kthread_t delay_thread = { attach, process, detach };


static ER_ID attach(void)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	T_CTSK pk_ctsk = {
		TA_HLNG, 0, process, pri_dispatcher,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	lfq_initialize(&(((system_info_t*)SYSTEM_INFO_ADDR)->kqueue),
			kqbuf, sizeof(delay_param_t), KQUEUE_SIZE);
	return info->delay_thread_id = thread_create_auto(&pk_ctsk);
}

static void process(VP_INT exinf)
{
	for (;;) {
		delay_param_t param;

		if (lfq_dequeue(&(((system_info_t*)SYSTEM_INFO_ADDR)->kqueue),
				&param) != QUEUE_OK)
			break;

		switch (param.action) {
		case delay_raise:
			raise((list_t*)(param.arg1));
			break;

		case delay_page_fault:
			kill((ID)(param.arg1));
			break;

		case delay_send:
			queue_send((ID)(param.arg1),
					(VP_INT)(param.arg2), TMO_POL);
			break;

		case delay_activate:
			thread_start((ID)(param.arg1));
			break;

		default:
			break;		
		}
	}
}

static void detach(void)
{
}

void kern_start(void)
{
	context_initialize();
	global_initialize();
	port_initialize();
	queue_initialize();
	thread_initialize();

	delay_thread.attach();
	arch_initialize();
	interrupt_initialize();
	timer_initialize();

	load_modules();

	for (;;)
		halt();
}

static ER kill(const int pid)
{
	ER_UINT rsize;
	struct posix_request req;

	req.caller = thread_get_id();
	//TODO define special process id
	req.procid = -1;
	req.operation = PSC_KILL;
	req.param.par_kill.pid = pid;

	rsize = port_call(PORT_FS, &req, sizeof(struct posix_request));
	return (rsize < 0)? rsize:E_OK;
}

static void raise(list_t *w)
{
	list_t guard;

	list_insert(w, &guard);

	while ((w = list_dequeue(&guard))) {
		thread_t *th = getThreadWaiting(w);
		void (*f)(thread_t*) =
				(void (*)(thread_t*))(th->wait.detail.dly.callback);

		f(th);
	}
}
