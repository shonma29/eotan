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
#include <mpufunc.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <nerve/func.h>
#include "ready.h"

typedef struct {
	VP_INT arg1;
	VP_INT arg2;
	VP_INT arg3;
	VP_INT arg4;
} svc_arg;

static void *page_alloc(void);
static void page_free(void *);
static ER region_get(const ID, const void *, const size_t, void *);
static ER region_put(const ID, void *, const size_t, const void *);
static ER_UINT region_copy(const ID, const void *, const size_t, void *);
static ER_UINT skip_copy(const ID, const size_t, copy_range_t *, const size_t);
static ER_UINT _ipc_call(svc_arg *);

static ER (*svc_entries[])(svc_arg *) = {
	_ipc_call
};


void kcall_initialize(void)
{
	kcall_t *p = (kcall_t *) KCALL_ADDR;

	p->dispatch = dispatch;
	p->tick = tick;
	p->thread_get_id = thread_get_id;
	p->thread_create = thread_create;
	p->thread_destroy = thread_destroy;
	p->thread_start = thread_start;
	p->thread_end_and_destroy = thread_end_and_destroy;
	p->thread_terminate = thread_terminate;
	p->thread_suspend = thread_suspend;
	p->thread_resume = thread_resume;
	p->palloc = page_alloc;
	p->pfree = page_free;
	p->printk = printk;

	p->region_get = region_get;
	p->region_put = region_put;
	p->region_copy = region_copy;
	p->skip_copy = skip_copy;

	p->ipc_open = ipc_open;
	p->ipc_close = ipc_close;
	p->ipc_call = ipc_call;
	p->ipc_receive = ipc_receive;
	p->ipc_send = ipc_send;
	p->ipc_listen = ipc_listen;
	p->ipc_notify = ipc_notify;
#ifdef USE_MUTEX
	p->mutex_create = mutex_create;
	p->mutex_destroy = mutex_destroy;
	p->mutex_lock = mutex_lock;
	p->mutex_unlock = mutex_unlock;
#endif
}

static void *page_alloc(void)
{
	enter_serialize();

	void *result = palloc();

	leave_serialize();
	return result;
}

static void page_free(void *addr)
{
	enter_serialize();

	pfree(addr);

	leave_serialize();
}

static ER region_get(const ID id, const void *from, const size_t size, void *to)
{
	enter_serialize();

	thread_t *th = get_thread_ptr(id);
	ER result = th ? memcpy_u2k(th, to, from, size) : E_NOEXS;

	leave_serialize();
	return result;
}

static ER region_put(const ID id, void *to, const size_t size, const void *from)
{
	enter_serialize();

	thread_t *th = get_thread_ptr(id);
	ER result = th ? memcpy_k2u(th, to, from, size) : E_NOEXS;

	leave_serialize();
	return result;
}

static ER_UINT region_copy(const ID id, const void *from, const size_t size, void *to)
{
	enter_serialize();

	thread_t *th = get_thread_ptr(id);
	ER_UINT result = th ? strncpy_u2k(th, to, from, size) : E_NOEXS;

	leave_serialize();
	return result;
}

static ER_UINT skip_copy(const ID id, const size_t skip, copy_range_t *ranges,
		const size_t n)
{
	enter_serialize();

	thread_t *th = get_thread_ptr(id);
	ER_UINT result = th ? scattered_copy_u2k(th, skip, ranges, n) : E_NOEXS;

	leave_serialize();
	return result;
}

ER syscall(svc_arg *argp, UW svcno)
{
	return ((svcno >= sizeof(svc_entries) / sizeof(svc_entries[0])) ?
			E_NOSPT : (svc_entries[svcno](argp)));
}

static ER_UINT _ipc_call(svc_arg *argp)
{
	return ipc_call((ID) (argp->arg1), (VP) (argp->arg2),
			(UINT) (argp->arg3));
}
