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
#include <services.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include <nerve/icall.h>
#include <nerve/kcall.h>
#include <sys/time.h>
#include <set/slab.h>
#include <set/tree.h>
#include <delay.h>
#include <sync.h>
#include <arch/8259a.h>
#include <arch/archfunc.h>
#include "../../lib/libserv/libserv.h"
#include "timer.h"

#define system_time (((system_info_t*)SYSTEM_INFO_ADDR)->system_time)

typedef struct {
	node_t node;
	struct timespec ts;
	list_t waiting;
} timer_t;

typedef struct {
	list_t brothers;
	RDVNO rdvno;
} sleeper_t;

static slab_t timer_slab;
static tree_t timer_tree;
static slab_t sleeper_slab;

static inline timer_t *getTimerParent(const node_t *p);
static inline sleeper_t *getSleeperParent(const list_t *p);
static void time_tick(void);
static int compare(const int a, const int b);
static void wakeup(list_t *w);
static ER add_timer(const struct timespec *ts, const RDVNO rdvno);
static void doit(void);
static ER init(void);


static inline timer_t *getTimerParent(const node_t *p) {
	return (timer_t*)((intptr_t)p - offsetof(timer_t, node));
}

static inline sleeper_t *getSleeperParent(const list_t *p) {
	return (sleeper_t*)((intptr_t)p - offsetof(sleeper_t, brothers));
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

static void wakeup(list_t *w)
{
	ER reply = E_TMOUT;
	list_t guard;

	list_insert(w, &guard);

	while ((w = list_dequeue(&guard))) {
		sleeper_t *s = getSleeperParent(w);
		ER result = kcall->port_reply(s->rdvno, &reply, sizeof(reply));
		if (result != E_OK)
			dbg_printf(MYNAME ": rpl_rdv(0x%x) failed %d\n",
					s->rdvno, result);
		slab_free(&sleeper_slab, s);
	}
}

static ER add_timer(const struct timespec *ts, const RDVNO rdvno)
{
	node_t *p;
	timer_t *t;
	timer_t entry;
	sleeper_t *s;

	if ((ts->tv_sec < 0) || (ts->tv_nsec < 0))
		return E_PAR;

	if ((ts->tv_sec == 0) && (ts->tv_nsec == 0))
		return E_TMOUT;

	time_get_raw(&(entry.ts));
	timespec_add(&(entry.ts), ts);

	enter_critical();
	s = slab_alloc(&sleeper_slab);
	if (!s) {
		leave_critical();
		return E_NOMEM;
	}

	list_initialize(&(s->brothers));
	s->rdvno = rdvno;

	p = tree_get(&timer_tree, (int)&entry);
	if (p)
		t = getTimerParent(p);
	else {
		p = tree_put(&timer_tree, (int)&entry);
		if (!p) {
			slab_free(&sleeper_slab, s);
			leave_critical();
			return E_NOMEM;
		}

		p->key = (int)p;
		t = getTimerParent(p);
		t->ts = entry.ts;
		list_initialize(&(t->waiting));
	}

	list_enqueue(&(t->waiting), &(s->brothers));
	leave_critical();

	return E_OK;
}

ER timer_service(void)
{
	struct timespec now;
	node_t *p;

	time_tick();
	kcall->thread_tick();

	time_get_raw(&now);

	while ((p = tree_first(&timer_tree))) {
		timer_t *t = getTimerParent(p);

		if (timespec_compare(&now, &(t->ts)) < 0)
			break;

		if (!list_is_empty(&(t->waiting))) {
			list_t *w = list_next(&(t->waiting));

			list_remove(&(t->waiting));
			icall->handle((void (*)(const int))wakeup, (int)w);
		}

		tree_remove(&timer_tree, (int)p);
	}

	return E_OK;
}

static void doit(void)
{
	for (;;) {
		struct timespec arg;
		RDVNO rdvno;
		ER reply;
		ER result;
		ER_UINT size = kcall->port_accept(PORT_TIMER, &rdvno, &arg);

		if (size < 0) {
			dbg_printf(MYNAME ": acp_por failed %d\n", size);
			break;
		}

		reply = (size == sizeof(arg))? add_timer(&arg, rdvno):E_PAR;
		if (reply != E_OK) {
			result = kcall->port_reply(rdvno, &reply,
					sizeof(reply));
			if (result != E_OK)
				dbg_printf(MYNAME ": rpl_rdv(0x%x) failed %d\n",
						rdvno, result);
		}
	}
}

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, sizeof(struct timespec), sizeof(ER) };
	T_DINH pk_dinh = { TA_HLNG, (FP)timer_service };
	ER result;

	timer_slab.unit_size = sizeof(timer_t);
	timer_slab.block_size = PAGE_SIZE;
	timer_slab.min_block = 1;
	timer_slab.max_block = tree_max_block(MAX_TIMER, PAGE_SIZE,
			sizeof(timer_t));
	timer_slab.palloc = kcall->palloc;
	timer_slab.pfree = kcall->pfree;
	slab_create(&timer_slab);
	tree_create(&timer_tree, &timer_slab, compare);

	sleeper_slab.unit_size = sizeof(sleeper_t);
	sleeper_slab.block_size = PAGE_SIZE;
	sleeper_slab.min_block = 1;
	sleeper_slab.max_block = MAX_TIMER / ((PAGE_SIZE - sizeof(slab_block_t))
			/ sizeof(sleeper_t));
	sleeper_slab.palloc = kcall->palloc;
	sleeper_slab.pfree = kcall->pfree;
	slab_create(&sleeper_slab);

	time_initialize();
	result = kcall->interrupt_bind(PIC_IR_VECTOR(ir_pit), &pk_dinh);
	if (result != E_OK) {
		dbg_printf(MYNAME ": interrupt_bind error=%d\n", result);
		return result;
	}

	pit_initialize(TIME_TICKS);

	result = kcall->interrupt_enable(ir_pit);
	if (result != E_OK) {
		dbg_printf(MYNAME ": interrupt_enable error=%d\n", result);
		pk_dinh.inthdr = NULL;
		kcall->interrupt_bind(PIC_IR_VECTOR(ir_pit), &pk_dinh);
		return result;
	}

	result = kcall->port_create(PORT_TIMER, &pk_cpor);
	if (result != E_OK) {
		dbg_printf(MYNAME ": cre_por failed %d\n", result);
		pk_dinh.inthdr = NULL;
		kcall->interrupt_bind(PIC_IR_VECTOR(ir_pit), &pk_dinh);
		return result;
	}

	return E_OK;
}

void start(VP_INT exinf)
{
	if (init() == E_OK) {
		ER error;

		dbg_printf(MYNAME ": start\n");
		doit();
		dbg_printf(MYNAME ": end\n");

		error = kcall->port_destroy(PORT_TIMER);
		if (error != E_OK)
			dbg_printf(MYNAME ": del_por failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
