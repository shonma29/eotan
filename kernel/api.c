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
#include <nerve/global.h>
#include <nerve/kcall.h>
#include "func.h"
#include "ready.h"
#include "arch/archfunc.h"
#include "mpu/mpufunc.h"

typedef struct {
	VP_INT arg1;
	VP_INT arg2;
	VP_INT arg3;
	VP_INT arg4;
} svc_arg;

static ER region_get(const ID id, const void *from, const size_t size,
		void *to);
static ER region_put(const ID id, void *to, const size_t size,
		const void *from);
static ER_UINT region_copy(const ID id, const void *from, const size_t size,
		void *to);
static ER_UINT _port_call(svc_arg *);

static ER (*svc_entries[])(svc_arg *) = {
	_port_call
};


void kcall_initialize(void)
{
	kcall_t *p = (kcall_t*)KCALL_ADDR;

	p->dispatch = dispatch;
	p->tick = tick;
	p->thread_get_id = thread_get_id;
	p->thread_create = thread_create;
	p->thread_create_auto = thread_create_auto;
	p->thread_destroy = thread_destroy;
	p->thread_start = thread_start;
	p->thread_end_and_destroy = thread_end_and_destroy;
	p->thread_terminate = thread_terminate;
	p->thread_sleep = thread_sleep;
	p->thread_wakeup = thread_wakeup;
	p->palloc = palloc;
	p->pfree = pfree;
	p->printk = printk;

	p->region_get = region_get;
	p->region_put = region_put;
	p->region_copy = region_copy;

	p->port_create = port_create;
	p->port_destroy = port_destroy;
	p->port_call = port_call;
	p->port_accept = port_accept;
	p->port_reply = port_reply;
	p->mutex_create = mutex_create;
	p->mutex_destroy = mutex_destroy;
	p->mutex_lock = mutex_lock;
	p->mutex_unlock = mutex_unlock;
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

ER syscall(svc_arg *argp, UW svcno)
{
	return (svcno >= sizeof(svc_entries) / sizeof(svc_entries[0]))?
			E_NOSPT:(svc_entries[svcno](argp));
}

static ER_UINT _port_call(svc_arg *argp)
{
	return port_call((ID)(argp->arg1), (VP)(argp->arg3), (UINT)(argp->arg4));
}
