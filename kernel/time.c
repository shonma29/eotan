/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * 時間管理機能
 */
/*
 * $Log: time.c,v $
 * Revision 1.4  2000/04/03 14:34:48  naniwa
 * to call timer handler in task
 *
 * Revision 1.3  2000/02/27 15:30:51  naniwa
 * to work as multi task OS
 *
 * Revision 1.2  2000/02/06 09:10:58  naniwa
 * minor fix
 *
 * Revision 1.1  1999/11/14 14:53:38  naniwa
 * add time management function
 *
 */

#include <core.h>
#include <string.h>
#include <sys/time.h>
#include "thread.h"
#include "func.h"
#include "ready.h"
#include "setting.h"
#include "sync.h"
#include "mpu/mpufunc.h"

static struct timer_list {
    struct timer_list *next;
    W time;
    void (*func) (VP);
    VP argp;
} timer[MAX_TIME_HANDLER];

/* 
 * 変数宣言
 */

/* 大域変数の宣言 */
BOOL do_timer = FALSE;

static struct timer_list *free_timer;
static struct timer_list *time_list;

static void set_timer(W time, void (*func) (VP), VP argp);
static ER unset_timer(void (*func) (VP), VP arg);


/*
 * タスク遅延
 */

static void
dly_func (VP p)
{
  thread_t *taskp;

  enter_serialize();
  taskp = (thread_t *)p;
  taskp->wait.result = E_OK;
  leave_serialize();

  release(taskp);
}

ER thread_delay(RELTIM dlytim)
{
  if (dlytim < 0) return(E_PAR);
  else if (! dlytim) return(E_OK);
  set_timer (dlytim, (void (*)(VP))dly_func, running);
  running->wait.type = wait_dly;
  wait(running);
  unset_timer ((void (*)(VP))dly_func, running);
  return (running->wait.result);
}


/*************************************************************************
 * intr_interval --- 
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	インターバルタイマの割り込み処理を行う。
 *
 */
void intr_interval(void)
{
    running->time.total++;

    /* システム時間の増加 */
    time_tick();

    if ((running->time.left) > 0 && (running->priority >= pri_user_foreground)) {
	if (--running->time.left == 0) {
	    running->time.left = TIME_QUANTUM;
	    ready_rotate(running->priority);
	    delayed_dispatch = TRUE;
	}
    }

    if (time_list != NULL) {
	(time_list->time)--;
	if ((time_list->time <= 0) && (do_timer == FALSE)) {
	    /* KERNEL_TASK で timer に設定されている関数を実行 */
	    do_timer = TRUE;
	    thread_start(delay_thread_id);
	    delayed_dispatch = TRUE;
	}
    }
}

/*************************************************************************
 * timer_initialize
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
void timer_initialize(void)
{
    W i;

    printk("initialize timer\n");
    enter_critical();
    for (i = 0; i <= MAX_TIME_HANDLER - 2; i++) {
	timer[i].next = &timer[i + 1];
    }
    timer[MAX_TIME_HANDLER - 1].next = NULL;
    free_timer = timer;
    time_list = NULL;
    leave_critical();
}

/*************************************************************************
 * set_timer 
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
static void set_timer(W time, void (*func) (VP), VP argp)
{
    struct timer_list *p, *q, *r;
    W total;

    if ((func == NULL) || (time <= 0)) {
	return;
    }
    enter_critical();
    p = free_timer;
    if (p == NULL) {
	printk("timer entry empty.\n");
	leave_critical();
	return;
    }
    free_timer = free_timer->next;
    leave_critical();
    p->time = time;
    p->func = (void (*)(VP)) func;
    p->argp = argp;
    p->next = NULL;

    enter_critical();
    if (time_list == NULL) {
	time_list = p;
	leave_critical();
	return;
    }

    total = time_list->time;
    if (total > p->time) {
	time_list->time -= p->time;
	p->next = time_list;
	time_list = p;
	leave_critical();
	return;
    }

    for (q = time_list, r = q->next; r != NULL; q = q->next, r = r->next) {
	if ((total + r->time) > p->time)
	    break;
	total += r->time;
    }

    p->time = p->time - total;
    p->next = r;
    q->next = p;
    if (r != NULL)
	r->time -= p->time;
    leave_critical();
}

/*************************************************************************
 * unset_timer --- タイマー待ち行列から、引数で指定した条件に合うエントリ
 *		   を削除する。
 *
 * 引数：
 *	func	
 *	arg
 *
 * 返値：
 *     エラー番号
 *
 * 処理：
 *	タイマーリストをチェックし、待ち時間のすぎたエントリがあれば、
 *	エントリにセットされた関数を呼び出す。
 *
 *
 */
static ER unset_timer(void (*func) (VP), VP arg)
{
    struct timer_list *point, *before;

    enter_critical();
    before = NULL;
    for (point = time_list; point != NULL; point = point->next) {
	if ((point->func == func) && (point->argp == arg)) {
	    break;
	}
	before = point;
    }
    if (point == NULL) {
	leave_critical();
	return (E_PAR);
    }

    if (point->next) {
	point->next->time += point->time;
    }
    if (before == NULL) {
	time_list = point->next;
    } else {
	before->next = point->next;
    }
    point->next = free_timer;
    free_timer = point;
    leave_critical();
    return (E_OK);
}

/*************************************************************************
 * check_timer
 *
 * 引数：
 *	なし
 *
 * 返値：
 *	なし
 *
 * 処理：
 *	タイマーリストをチェックし、待ち時間のすぎたエントリがあれば、
 *	エントリにセットされた関数を呼び出す。
 *
 */
void check_timer(void)
{
    struct timer_list *p, *q;

    if (time_list == NULL) {
	return;
    }

    for (p = time_list; (p != NULL) && (p->time <= 0L); p = q) {
	(p->func) (p->argp);
	q = p->next;
	if (q != NULL)
	    q->time += time_list->time;
	p->next = free_timer;
	enter_critical();
	free_timer = p;
	time_list = q;
	leave_critical();
    }
}
