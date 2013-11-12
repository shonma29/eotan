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
#include <global.h>
#include <sys/time.h>
#include "delay.h"
#include "func.h"
#include "ready.h"
#include "setting.h"
#include "sync.h"
#include "mpu/mpufunc.h"

#define TICK (1000 * 1000 * 1000 / TIME_TICKS)

typedef struct {
	node_t node;
	struct timespec ts;
	list_t threads;
} timer_t;

static struct timespec system_time;
static slab_t timer_slab;
static tree_t timer_tree;

static inline timer_t *getTimerParent(const node_t *p);
static ER add_timer(RELTIM time, thread_t *th);


static inline timer_t *getTimerParent(const node_t *p) {
	return (timer_t*)((ptr_t)p - offsetof(timer_t, node));
}

void time_initialize(time_t *seconds)
{
	long nsec = 0;

	timespec_set(&system_time, seconds, &nsec);
}

ER time_set(SYSTIM *pk_systim)
{
	if (!pk_systim)
		return E_PAR;

//TODO check like time_get, or disable interrupt
	timespec_set(&system_time, &(pk_systim->sec), &(pk_systim->nsec));

	return E_OK;
}

ER time_get(SYSTIM *pk_systim)
{
	struct timespec t1;
	struct timespec t2;

	if (!pk_systim)
		return E_PAR;

	do {
		t1 = system_time;
		t2 = system_time;
	} while (!timespec_equals(&t1, &t2));

	timespec_get_sec(&(pk_systim->sec), &t1);
	timespec_get_nsec(&(pk_systim->nsec), &t1);

	return E_OK;
}

void time_get_raw(struct timespec *ts)
{
	struct timespec t2;

	do {
		*ts = system_time;
		t2 = system_time;
	} while (!timespec_equals(ts, &t2));
}

void time_tick(void)
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
	timer_slab.unit_size = sizeof(timer_t);
	timer_slab.block_size = PAGE_SIZE;
	timer_slab.min_block = 1;
	timer_slab.max_block = tree_max_block(65536, PAGE_SIZE,
			sizeof(timer_t));
	timer_slab.palloc = palloc;
	timer_slab.pfree = pfree;
	slab_create(&timer_slab);

	tree_create(&timer_tree, &timer_slab, compare);
}

static void resume(thread_t *th)
{
	list_remove(&(th->wait.waiting));
	release(th);
}

ER thread_delay(RELTIM dlytim)
{
	ER result;

	if (dlytim < 0)
		return E_PAR;

	else if (!dlytim)
		return E_OK;

	result = add_timer(dlytim, running);
	if (result)
		return result;

	running->wait.type = wait_dly;
	running->wait.detail.dly.callback = (FP)resume;
	wait(running);

	return running->wait.result;
}

static ER add_timer(RELTIM time, thread_t *th)
{
	node_t *p;
	timer_t *t;
	timer_t entry;
	struct timespec add;

	if (time <= 0)
		return E_PAR;

	add.tv_sec = time / (1000 * 1000);
	add.tv_nsec = time % (1000 * 1000);
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
	running->time.total++;

	if (running->attr.domain_id != KERNEL_DOMAIN_ID)
		if (!(--(running->time.left))) {
			running->time.left = TIME_QUANTUM;
			ready_rotate(running->priority);
		}

	time_get_raw(&now);

	while ((p = tree_first(&timer_tree))) {
		timer_t *t = getTimerParent(p);

		if (timespec_compare(&now, &(t->ts)) < 0)
			break;

		if (!list_is_empty(&(t->threads))) {
			list_t *w = list_next(&(t->threads));
			delay_param_t param;
			system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

			list_remove(&(t->threads));

			param.action = delay_raise;
			param.arg1 = (int)w;

			if (lfq_enqueue(&(info->kqueue),
					&param) != QUEUE_OK)
	    			panic("full kqueue");

			//TODO prevent other handlers from releasing threads
			thread_start(info->delay_thread_id);
    		}

		tree_remove(&timer_tree, (int)p);
	}

	return E_OK;
}
