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
#include <nerve/config.h>
#include <nerve/global.h>
#include <sys/time.h>
#include "delay.h"
#include "func.h"
#include "ready.h"
#include "sync.h"
#include "arch/archfunc.h"
#include "mpu/mpufunc.h"

#define TICK (1000 * 1000 * 1000 / TIME_TICKS)

typedef struct {
	node_t node;
	struct timespec ts;
	list_t threads;
} timer_t;

#define system_time (((system_info_t*)SYSTEM_INFO_ADDR)->system_time)

static slab_t timer_slab;
static tree_t timer_tree;

static inline timer_t *getTimerParent(const node_t *p);
static void time_initialize(void);
static void time_tick(void);
static int compare(const int a, const int b);
static void wakeup(list_t *w);
static ER add_timer(const TMO usec, thread_t *th);


static inline timer_t *getTimerParent(const node_t *p) {
	return (timer_t*)((intptr_t)p - offsetof(timer_t, node));
}

static void time_initialize(void)
{
	long nsec = 0;
	time_t seconds;

	rtc_get_time(&seconds);
	timespec_set(&system_time, &seconds, &nsec);
}

static void time_tick(void)
{
	struct timespec add = {
		0, TICK
	};

	timespec_add(&system_time, &add);
}

static int compare(const int a, const int b)
{
	struct timespec *t1 = (struct timespec*)&(((timer_t*)a)->ts);
	struct timespec *t2 = (struct timespec*)&(((timer_t*)b)->ts);

	return timespec_compare(t1, t2);
}

void timer_initialize(void)
{
	create_tree(&timer_tree, &timer_slab, sizeof(timer_t), compare);
	time_initialize();
	pit_initialize(TIME_TICKS);
}

static void wakeup(list_t *w)
{
	list_t guard;

	list_insert(w, &guard);

	while ((w = list_dequeue(&guard))) {
		thread_t *th = getThreadWaiting(w);

		th->wait.result = E_TMOUT;
		list_remove(&(th->wait.waiting));
		release(th);
	}
}

ER thread_sleep(TMO tmout)
{
	if (tmout < TMO_FEVR)
		return E_PAR;

	if (running->wakeup_count > 0) {
		running->wakeup_count--;
		return E_OK;
	}

	if (tmout == TMO_POL)
		return E_TMOUT;

	else if (tmout > 0) {
		ER result = add_timer(tmout, running);
		if (result)
			return result;
	}

	running->wait.type = wait_slp;
	wait(running);

	return running->wait.result;
}

static ER add_timer(const TMO usec, thread_t *th)
{
	node_t *p;
	timer_t *t;
	timer_t entry;
	struct timespec add;

	if (usec <= 0)
		return E_PAR;

	add.tv_sec = usec / (1000 * 1000);
	add.tv_nsec = (usec % (1000 * 1000)) * 1000;
	entry.node.key = (int)&t;
	time_get_raw(&(entry.ts));
	timespec_add(&(entry.ts), &add);

	enter_critical();
	p = tree_get(&timer_tree, (int)&entry);
	if (!p) {
		p = tree_put(&timer_tree, (int)&entry);
		if (!p) {
			leave_critical();
			return E_NOMEM;
		}

		p->key = (int)p;
		t = getTimerParent(p);
		t->ts = entry.ts;
		list_initialize(&(t->threads));
	}
	else
		t = getTimerParent(p);

	list_enqueue(&(t->threads), &(th->wait.waiting));
	leave_critical();

	return E_OK;
}

ER timer_service(void)
{
	struct timespec now;
	node_t *p;

	time_tick();
	thread_tick();

	time_get_raw(&now);

	while ((p = tree_first(&timer_tree))) {
		timer_t *t = getTimerParent(p);

		if (timespec_compare(&now, &(t->ts)) < 0)
			break;

		if (!list_is_empty(&(t->threads))) {
			list_t *w = list_next(&(t->threads));
			delay_param_t param;

			list_remove(&(t->threads));

			param.action = delay_handle;
			param.arg1 = (int)wakeup;
			param.arg2 = (int)w;

			if (kq_enqueue(&param))
				panic("full kqueue");
		}

		tree_remove(&timer_tree, (int)p);
	}

	return E_OK;
}
