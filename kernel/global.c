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
#include <fstype.h>
#include <major.h>
#include <nerve/global.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include "delay.h"
#include "func.h"
#include "ready.h"
#include "arch/archfunc.h"
#include "mpu/mpufunc.h"

static void kcall_initialize(void);
static void icall_initialize(void);
static ER delayed_thread_start(ID thread_id);
static ER delayed_queue_send_nowait(ID queue_id, VP_INT data);
static ER delayed_handle(void (*callback)(void));
static ER region_get(const ID id, const void *from, const size_t size,
		void *to);
static ER region_put(const ID id, void *to, const size_t size,
		const void *from);
static ER_UINT region_copy(const ID id, const void *from, const size_t size,
		void *to);


void global_initialize(void)
{
	system_info_t *sysinfo = (system_info_t *)SYSTEM_INFO_ADDR;

	sysinfo->root.device = get_device_id(DEVICE_MAJOR_ATA, 0);
	sysinfo->root.fstype = FS_SFS;
	sysinfo->initrd.start = 0;
	sysinfo->initrd.size = 0;
	sysinfo->delay_thread_id = TSK_NONE;

	kcall_initialize();
	icall_initialize();
}

static void kcall_initialize(void)
{
	kcall_t *p = (kcall_t*)KCALL_ADDR;

	p->dispatch = dispatch;
	p->thread_create_auto = thread_create_auto;
	p->thread_destroy = thread_destroy;
	p->thread_start = thread_start;
	p->thread_end_and_destroy = thread_end_and_destroy;
	p->thread_terminate = thread_terminate;
	p->time_get = time_get;
	p->time_set = time_set;
	p->interrupt_bind = interrupt_bind;
	p->interrupt_enable = pic_reset_mask;
	p->palloc = palloc;
	p->pfree = pfree;
	p->puts = putsk;

	p->region_get = region_get;
	p->region_put = region_put;
	p->region_copy = region_copy;

	p->port_create = port_create;
	p->port_create_auto = port_create_auto;
	p->port_destroy = port_destroy;
	p->port_call = port_call;
	p->port_accept = port_accept;
	p->port_reply = port_reply;
	p->queue_create_auto = queue_create_auto;
	p->queue_destroy = queue_destroy;
	p->queue_send = queue_send;
	p->queue_receive = queue_receive;
}

static void icall_initialize(void)
{
	icall_t *p = (icall_t*)ICALL_ADDR;

	p->thread_start = delayed_thread_start;
	p->thread_get_id = thread_get_id;
	p->queue_send_nowait = delayed_queue_send_nowait;
	p->handle = delayed_handle;
	p->puts = putsk;
}

static ER delayed_thread_start(ID thread_id)
{
	delay_param_t param;

	param.action = delay_start;
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

static ER delayed_handle(void (*callback)(void))
{
	delay_param_t param;

	param.action = delay_handle;
	param.arg1 = (int)callback;

	return kq_enqueue(&param);
}

ER kq_enqueue(delay_param_t *param)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (lfq_enqueue(&(info->kqueue), param) != QUEUE_OK)
		return E_TMOUT;

	delay_start = TRUE;

	return E_OK;
}

static ER region_get(const ID id, const void *from, const size_t size, void *to)
{
	thread_t *th = get_thread_ptr(id);

	return th? copy_from(th, to, from, size):E_NOEXS;
}

static ER region_put(const ID id, void *to, const size_t size, const void *from)
{
	thread_t *th = get_thread_ptr(id);

	return th? copy_to(th, to, from, size):E_NOEXS;
}

static ER_UINT region_copy(const ID id, const void *from, const size_t size, void *to)
{
	thread_t *th = get_thread_ptr(id);

	return th? ncpy_from(th, to, from, size):E_NOEXS;
}
