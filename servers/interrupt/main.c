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
#include <interrupt.h>
#include <services.h>
#include <nerve/config.h>
#include <nerve/delay.h>
#include <nerve/global.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include <arch/archfunc.h>
#include <mpu/mpufunc.h>
#include <set/lf_queue.h>
#include "../../lib/libserv/libserv.h"
#include "interrupt_service.h"

static char kqbuf[lfq_buf_size(sizeof(delay_param_t), KQUEUE_SIZE)];

static void icall_initialize(void);
static ER delayed_handle(void (*callback)(const int, const int), int, int);
static ER kq_enqueue(delay_param_t *);
static void delay_process(void);
static ER delay_initialize(void);
static ER port_initialize(void);
static ER execute(int_args_t *);
static ER accept(void);


static void icall_initialize(void)
{
	icall_t *p = (icall_t *) ICALL_ADDR;
	p->thread_get_id = kcall->thread_get_id;//TODO why here?
	p->handle = delayed_handle;
}

static ER delayed_handle(void (*callback)(const int arg1, const int arg2),
		int arg1, int arg2)
{
	delay_param_t param = {
		(int) callback,
		arg1,
		arg2
	};
	return kq_enqueue(&param);
}

static ER kq_enqueue(delay_param_t *param)
{
	if (lfq_enqueue(&(sysinfo->kqueue), param) != QUEUE_OK)
		return E_TMOUT;

	//TODO not boolean. pass PORT_DELAY to wakeup.
	sysinfo->delay_thread_start = true;
	return E_OK;
}

static void delay_process(void)
{
	for (;;) {
		delay_param_t param;
		if (lfq_dequeue(&(sysinfo->kqueue), &param) == QUEUE_OK) {
			((void (*)(const int, const int))(param.arg1))(
					(const int) (param.arg2),
					(const int) (param.arg3));
		} else
			kcall->ipc_listen();
	}
}

static ER delay_initialize(void)
{
	lfq_initialize(&(sysinfo->kqueue),
			kqbuf, sizeof(delay_param_t), KQUEUE_SIZE);

	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, delay_process, pri_dispatcher,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};
	ER result = kcall->thread_create(PORT_DELAY, &pk_ctsk);
	if (result) {
		log_err(MYNAME ": create error=%d\n", result);
		return result;
	}

	icall_initialize();
	arch_initialize();
	interrupt_initialize();
	return E_OK;
}

static ER port_initialize(void)
{
	T_CPOR pk_cpor = {
		TA_TFIFO,
		sizeof(int_args_t),
		sizeof(ER)
	};
	W result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err("interrupt: open error=%d\n", result);
		return result;
	}

	return E_OK;
}

static ER execute(int_args_t *args)
{
	switch (args->operation) {
	case int_operation_bind:
		return interrupt_bind((T_CISR *) (args->arg1));
	case int_operation_unbind:
		return interrupt_unbind((ID) (args->arg1));
	case int_operation_enable:
		return pic_reset_mask((UB) (args->arg1));
	default:
		break;
	}

	return E_NOSPT;
}

static ER accept(void)
{
	int_args_t args;
	int tag;
	int size = kcall->ipc_receive(PORT_INTERRUPT, &tag, &args);
	if (size < 0) {
		log_err("interrupt: receive error=%d\n", size);
		return size;
	}

	int *reply = (int *) &args;
	*reply = (size == sizeof(args)) ? execute(&args) : E_PAR;

	int result = kcall->ipc_send(tag, &args, sizeof(*reply));
	if (result)
		log_err("interrupt: reply error=%d\n", result);

	return result;
}

void start(VP_INT exinf)
{
	if (delay_initialize() == E_OK)
		if (port_initialize() == E_OK) {
			log_info(MYNAME ": start\n");

			for (;;)
				accept();

			log_info(MYNAME ": end\n");
		}

	kcall->thread_end_and_destroy();
	//TODO disable interrupt
}
