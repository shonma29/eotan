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
#include <archfunc.h>
#include <nerve/config.h>
#include <nerve/delay.h>
#include <nerve/global.h>
#include <nerve/sync.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include <sys/time.h>
#include <set/slab.h>
#include <set/tree.h>
#include <arch/8259a.h>
#include "../../lib/libserv/libserv.h"
#include "timer.h"

#define system_time (((system_info_t *) SYSTEM_INFO_ADDR)->system_time)

typedef struct {
	node_t node;
	struct timespec ts;
	list_t waiting;
} timer_t;

typedef struct {
	list_t brothers;
	int tag;
} sleeper_t;

static slab_t timer_slab;
static tree_t timer_tree;
static slab_t sleeper_slab;

static inline timer_t *getTimerParent(const node_t *);
static inline sleeper_t *getSleeperParent(const list_t *);
static void time_tick(void);
static int compare(const int, const int);
static void wakeup(list_t *, int);
static ER add_timer(const struct timespec *, const int);
static void doit(void);
static ER init(void);


static inline timer_t *getTimerParent(const node_t *p)
{
	return ((timer_t *) ((uintptr_t) p - offsetof(timer_t, node)));
}

static inline sleeper_t *getSleeperParent(const list_t *p)
{
	return ((sleeper_t *) ((uintptr_t) p
			- offsetof(sleeper_t, brothers)));
}

static void time_initialize(void)
{
	const static int32_t nsec = 0;
	time_t seconds;
	rtc_get_time(&seconds);
	timespec_set(&system_time, &seconds, &nsec);
}

static void time_tick(void)
{
	const static struct timespec add = { 0, TICK };
	timespec_add(&system_time, &add);
}

static int compare(const int a, const int b)
{
	struct timespec *t1 = (struct timespec *) &(((timer_t *) a)->ts);
	struct timespec *t2 = (struct timespec *) &(((timer_t *) b)->ts);
	return timespec_compare(t1, t2);
}

static void wakeup(list_t *w, int dummy)
{
	const static ER reply = E_TMOUT;
	list_t guard;
	list_append(w, &guard);

	while ((w = list_dequeue(&guard))) {
		sleeper_t *s = getSleeperParent(w);
		ER result = kcall->ipc_send(s->tag, &reply, sizeof(reply));
		if (result != E_OK)
			kcall->printk(MYNAME ": reply(0x%x) failed %d\n",
					s->tag, result);

		slab_free(&sleeper_slab, s);
	}
}

static ER add_timer(const struct timespec *ts, const int tag)
{
	if ((ts->tv_sec < 0)
			|| (ts->tv_nsec < 0))
		return E_PAR;

	if ((ts->tv_sec == 0)
			&& (ts->tv_nsec == 0))
		return E_TMOUT;

	timer_t entry;
	time_get_raw(&(entry.ts));
	timespec_add(&(entry.ts), ts);

	enter_critical();
	sleeper_t *s = slab_alloc(&sleeper_slab);
	if (!s) {
		leave_critical();
		return E_NOMEM;
	}

	list_initialize(&(s->brothers));
	s->tag = tag;

	timer_t *t;
	node_t *p = tree_get(&timer_tree, (int) &entry);
	if (p)
		t = getTimerParent(p);
	else {
		p = slab_alloc(&timer_slab);
		if (!p) {
			slab_free(&sleeper_slab, s);
			leave_critical();
			return E_NOMEM;
		}

		if (!tree_put(&timer_tree, (int) &entry, p)) {
			slab_free(&timer_slab, p);
			slab_free(&sleeper_slab, s);
			leave_critical();
			return E_SYS;
		}

		p->key = (int) p;
		t = getTimerParent(p);
		t->ts = entry.ts;
		list_initialize(&(t->waiting));
	}

	list_enqueue(&(t->waiting), &(s->brothers));
	leave_critical();

	return E_OK;
}

void timer_service(VP_INT exinf)
{
	time_tick();
	kcall->tick();

	struct timespec now;
	time_get_raw(&now);

	for (node_t *p; (p = tree_first(&timer_tree));) {
		timer_t *t = getTimerParent(p);
		if (timespec_compare(&now, &(t->ts)) < 0)
			break;

		if (!list_is_empty(&(t->waiting))) {
			list_t *w = list_next(&(t->waiting));

			list_remove(&(t->waiting));
			icall->handle((void (*)(const int, const int)) wakeup,
					(int) w, 0);
		}

		p = tree_remove(&timer_tree, (int) p);
		if (p)
			slab_free(&timer_slab, p);
	}
}

static void doit(void)
{
	for (;;) {
		struct timespec arg;
		int tag;
		int size = kcall->ipc_receive(PORT_TIMER, &tag, &arg);
		if (size < 0) {
			kcall->printk(MYNAME ": receive failed %d\n", size);
			break;
		}

		int reply = (size == sizeof(arg)) ?
				add_timer(&arg, tag) : E_PAR;
		if (reply != E_OK) {
			int result = kcall->ipc_send(tag, &reply,
					sizeof(reply));
			if (result != E_OK)
				kcall->printk(MYNAME ": reply(0x%x) failed %d\n",
						tag, result);
		}
	}
}

static ER init(void)
{
	timer_slab.unit_size = sizeof(timer_t);
	timer_slab.block_size = PAGE_SIZE;
	timer_slab.min_block = 1;
	timer_slab.max_block = slab_max_block(MAX_TIMER, PAGE_SIZE,
			sizeof(timer_t));
	timer_slab.palloc = kcall->palloc;
	timer_slab.pfree = kcall->pfree;
	slab_create(&timer_slab);
	tree_create(&timer_tree, compare, NULL);

	sleeper_slab.unit_size = sizeof(sleeper_t);
	sleeper_slab.block_size = PAGE_SIZE;
	sleeper_slab.min_block = 1;
	sleeper_slab.max_block = slab_max_block(MAX_TIMER, PAGE_SIZE,
			sizeof(sleeper_t));
	sleeper_slab.palloc = kcall->palloc;
	sleeper_slab.pfree = kcall->pfree;
	slab_create(&sleeper_slab);

	time_initialize();

	T_CISR pk_cisr = {
		TA_HLNG,
		PIC_IR_VECTOR(ir_pit),
		PIC_IR_VECTOR(ir_pit),
		timer_service
	};
	ER_ID id = create_isr(&pk_cisr);
	if (id < 0) {
		kcall->printk(MYNAME ": define_handler error=%d\n", id);
		return id;
	}

	pit_initialize(TIME_TICKS);

	ER result = enable_interrupt(ir_pit);
	if (result != E_OK) {
		kcall->printk(MYNAME ": interrupt_enable error=%d\n", result);
		destroy_isr(id);
		return result;
	}

	T_CPOR pk_cpor = { TA_TFIFO, sizeof(struct timespec), sizeof(ER) };
	result = kcall->ipc_open(&pk_cpor);
	if (result != E_OK) {
		kcall->printk(MYNAME ": open failed %d\n", result);
		destroy_isr(id);
		return result;
	}

	return E_OK;
}

void start(VP_INT exinf)
{
	if (init() == E_OK) {
		kcall->printk(MYNAME ": start\n");
		doit();
		kcall->printk(MYNAME ": end\n");

		ER error = kcall->ipc_close();
		if (error != E_OK)
			kcall->printk(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
