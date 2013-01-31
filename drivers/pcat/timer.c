/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* timer.c
 *
 *
 */

#include <mpu/io.h>
#include "core.h"
#include "thread.h"
#include "func.h"
#include "sync.h"
#include "../../kernel/mpu/interrupt.h"

#define TIMER0_WRITE	0x40
#define TIMER0_READ	0x40
#define TIMER_CONTROL	0x43

/* PC/AT のタイマ・クロック周波数 */
#define TIMER_FREQ	1193182L

/* 大域変数の宣言 */
W do_timer = 0;

static struct timer_list {
    struct timer_list *next;
    W time;
    void (*func) (VP);
    VP argp;
} timer[MAX_TIMER];

static struct timer_list *free_timer;
static struct timer_list *time_list;
static unsigned long free_run;	/* 今の所初期化する予定なし */


static void write_vram(W, W, W, W);


/*************************************************************************
 * start_interval --- インターバルタイマの初期化
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	インターバルタイマを初期化する。
 *		10 ms ごとに割り込みを発生させるようにする。
 *
 */
void start_interval(void)
{
#if 0
    W i;
#endif

    printk("start interval\n");
    /* timer
     */
    set_idt(32, 0x08, (int) int32_handler, INTERRUPT_DESC, 0);
    reset_intr_mask(0);

    enter_critical();
#ifdef notdef
    outb(TIMER_CONTROL, 0x34);
    outb(TIMER0_WRITE, 11930 & 0xff);
    outb(TIMER0_WRITE, (11930 >> 8) & 0xff);
#else
    outb(TIMER_CONTROL, 0x36);	/* MODE 3 */
    outb(TIMER0_WRITE, (TIMER_FREQ/TICKS) & 0xff);
    outb(TIMER0_WRITE, ((TIMER_FREQ/TICKS) >> 8) & 0xff);
#endif
    leave_critical();

    write_vram(78, 0, 'x', 0);
}

#ifdef notdef
outb(W addr, unsigned char data)
{
    if ((addr == 0x40) || (addr == 0x43)) {
	printk("Warning!! outb (0x%x) output!! (data is %d)\n", addr,
	       data);
	return;
    }
    outb2(addr, data);
}
#endif


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
    SYSTIME time;
    UW TH, TM, TL;

    run_task->total++;

    /* システム時間の増加 */
    time_get(&time);
    TH = time.utime;
    TM = time.ltime >> 16;
    TL = time.ltime & 0x0FFFF;
    TL += CLOCK;
    TM += TL >> 16;
    time.ltime = ((TM & 0x0FFFF) << 16) + (TL & 0x0FFFF);
    time.utime = TH + (TM >> 16);
    time_set(&time);

    if ((run_task->quantum) > 0 && (run_task->tsklevel >= USER_LEVEL)) {
	if (--run_task->quantum == 0) {
	    run_task->quantum = QUANTUM;
	    rot_rdq(TPRI_RUN);
#ifdef notdef
	    /* rot_rdq 内部でも呼び出される */
	    thread_switch();
#endif
	}
    }

    if (time_list != NULL) {
	(time_list->time)--;
	if ((time_list->time <= 0) && (do_timer == 0)) {
	    /* KERNEL_TASK で timer に設定されている関数を実行 */
	    do_timer = 1;
	    thread_change_priority(KERNEL_TASK, MIN_PRIORITY);
	    thread_switch();
	}
    }
}

/*************************************************************************
 * init_timer
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
#if 0
    static struct timer_list banhei = { NULL, 0, 0, NULL };
#endif

    printk("** init_timer **\n");
    enter_critical();
    for (i = 0; i <= MAX_TIMER - 2; i++) {
	timer[i].next = &timer[i + 1];
    }
    timer[MAX_TIMER - 1].next = NULL;
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
void set_timer(W time, void (*func) (VP), VP argp)
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
ER unset_timer(void (*func) (VP), VP arg)
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

#ifdef notdef
    enter_critical();
#endif
    if (time_list == NULL) {
#ifdef notdef
	leave_critical();
#endif
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

#define TEXT_VRAM_ADDR	0x800B8000	/* TEXT VRAM のアドレス         */
#define CURSOR_POS(x,y)		(x + y * 80)

static void write_vram(W x, W y, W ch, W attr)
{
    H *addr;
#if 0
    H *attr_addr;
#endif

    addr = (H *) TEXT_VRAM_ADDR;
    ch = ch | (addr[CURSOR_POS(x, y)] & 0xff00);
    addr[CURSOR_POS(x, y)] = ch;
}

/* left_time()
 */

W left_time(void (*func) (VP), VP arg)
{
    struct timer_list *point;
    W time;

    time = 0;
    for (point = time_list; point != NULL; point = point->next) {
	time += point->time;
	if ((point->func == func) && (point->argp == arg)) {
	    break;
	}
    }
    return (time);
}

UW get_free_run(void)
{
    return free_run;
}
