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
#include <delay.h>
#include <services.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include <arch/archfunc.h>
#include <mpu/mpufunc.h>
#include <set/lf_queue.h>
#include <sys/syscall.h>
#include "../../lib/libserv/libserv.h"
#include "interrupt.h"

static char kqbuf[lfq_buf_size(sizeof(delay_param_t), KQUEUE_SIZE)];

static void icall_initialize(void);
static ER delayed_thread_start(ID thread_id);
static ER delayed_queue_send_nowait(ID queue_id, VP_INT data);
static ER delayed_handle(void (*callback)(const int arg), int arg);
static ER delayed_kill(int pid);
static ER kq_enqueue(delay_param_t *param);
static ER kill(const int pid);
static void delay_process(void);
static ER init(void);


static void icall_initialize(void)
{
	icall_t *p = (icall_t*)ICALL_ADDR;

	p->thread_start = delayed_thread_start;
	p->queue_send_nowait = delayed_queue_send_nowait;
	p->handle = delayed_handle;
	p->kill = delayed_kill;
}

static ER delayed_thread_start(ID thread_id)
{
	delay_param_t param;

	param.action = delay_activate;
	param.arg1 = (int)thread_id;

	return kq_enqueue(&param);
}

static ER delayed_queue_send_nowait(ID queue_id, VP_INT data)
{
	delay_param_t param;

	param.action = delay_send;
	param.arg1 = (int)queue_id;
	param.arg2 = (int)data;

	return kq_enqueue(&param);
}

static ER delayed_handle(void (*callback)(const int arg), int arg)
{
	delay_param_t param;

	param.action = delay_handle;
	param.arg1 = (int)callback;
	param.arg2 = arg;

	return kq_enqueue(&param);
}

static ER delayed_kill(int pid)
{
	delay_param_t param;

	param.action = delay_page_fault;
	param.arg1 = pid;

	return kq_enqueue(&param);
}

static ER kq_enqueue(delay_param_t *param)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (lfq_enqueue(&(info->kqueue), param) != QUEUE_OK)
		return E_TMOUT;

	sysinfo->delay_thread_start = TRUE;

	return E_OK;
}

static ER kill(const int pid)
{
	ER_UINT rsize;
	struct posix_request req;

	//TODO define special process id
	req.procid = -1;
	req.operation = PSC_KILL;
	req.args.arg1 = (W)pid;
	req.args.arg2 = 9;

	rsize = kcall->port_call(PORT_FS, &req, sizeof(struct posix_request));
	return (rsize < 0)? rsize:E_OK;
}

static void delay_process(void)
{
	for (;;) {
		for (;;) {
			delay_param_t param;

			if (lfq_dequeue(&(sysinfo->kqueue), &param) != QUEUE_OK)
				break;

			switch (param.action) {
			case delay_handle:
				((void (*)(const int))(param.arg1))
						((const int)(param.arg2));
				break;

			case delay_page_fault:
				kill((ID)(param.arg1));
				break;

			case delay_send:
				kcall->queue_send((ID)(param.arg1),
						(VP_INT)(param.arg2), TMO_POL);
				break;

			case delay_activate:
				kcall->thread_start((ID)(param.arg1));
				break;

			default:
				break;
			}
		}

		kcall->thread_sleep();
	}
}

static ER init(void)
{
	ER result;
	ER_ID tid;
	T_CTSK pk_ctsk = {
		TA_HLNG, 0, delay_process, pri_dispatcher,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	lfq_initialize(&(sysinfo->kqueue),
			kqbuf, sizeof(delay_param_t), KQUEUE_SIZE);

	tid = kcall->thread_create_auto(&pk_ctsk);
	if (tid < 0) {
		dbg_printf(MYNAME ": acre_tsk failed %d\n", tid);
		return tid;
	}

	result = kcall->thread_start(tid);
	if (result) {
		dbg_printf(MYNAME ": sta_tsk failed %d\n", result);
		kcall->thread_destroy(tid);
		return result;
	}

	sysinfo->delay_thread_id = tid;
	icall_initialize();
	return E_OK;
}

void start(VP_INT exinf)
{
	if (init() == E_OK) {
		dbg_printf(MYNAME ": start\n");
		kcall->thread_sleep();
		dbg_printf(MYNAME ": end\n");
	}

	kcall->thread_end_and_destroy();
}
