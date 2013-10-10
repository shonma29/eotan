/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* task.c $Revision: 1.16 $
 *
 * タスク管理を行う。
 *
 */
/*
 * $Log: task.c,v $
 * Revision 1.16  2000/04/23 02:18:41  naniwa
 * fix rel_wai
 *
 * Revision 1.15  2000/04/03 14:34:47  naniwa
 * to call timer handler in task
 *
 * Revision 1.14  2000/02/27 15:30:50  naniwa
 * to work as multi task OS
 *
 * Revision 1.13  2000/02/06 09:10:56  naniwa
 * minor fix
 *
 * Revision 1.12  2000/01/26 08:29:14  naniwa
 * minor fix
 *
 * Revision 1.11  1999/11/19 10:13:06  naniwa
 * fixed usage of add/del_tcb_list
 *
 * Revision 1.10  1999/11/10 10:29:08  naniwa
 * to support execve, etc
 *
 * Revision 1.9  1999/07/30 08:18:32  naniwa
 * add vcpy_stk()
 *
 * Revision 1.8  1999/07/09 08:19:50  naniwa
 * supported del_tsk
 *
 * Revision 1.7  1999/04/12 13:29:27  monaka
 * printf() is renamed to printk().
 *
 * Revision 1.6  1999/03/16 13:02:54  monaka
 * Modifies for source cleaning. Most of these are for avoid gcc's -Wall message.
 *
 * Revision 1.5  1998/02/24 14:08:26  night
 * sta_tsk() の変更。
 * sta_tsk() の第2引数 stacd が 0 よりも大きい時には、
 * スタックのトップアドレスを stacd の値分小さくする
 * するようにした。
 *
 * Revision 1.4  1997/10/11 16:21:41  night
 * こまごました修正
 *
 * Revision 1.3  1997/07/02 13:25:05  night
 * sta_tsk 実行時のデバッグ文を削除
 *
 * Revision 1.2  1997/03/25 13:30:47  night
 * 関数のプロトタイプ宣言の追加および引数の不整合の修正
 *
 * Revision 1.1  1996/07/22  13:39:15  night
 * IBM PC 版 ITRON の最初の登録
 *
 * Revision 1.13  1995/10/01  12:58:37  night
 * 関数 wup_tsk() の中で wakeup するタスクの情報のアドレスのチェックを追
 * 加。
 *
 * Revision 1.12  1995/09/21  15:51:12  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.11  1995/09/17  16:57:07  night
 * task_switch() 呼び出し時の処理の変更。
 * カレントタスクより優先順位が高いタスクがいない場合タスクスイッチを行わ
 * なかったが、優先順位の低いタスクへも切りかえるように変更した。
 *
 * Revision 1.10  1995/09/14  04:32:05  night
 * タスク状態を表示する関数 (print_list()) の変更。
 * 表示内容にタスクレベルを追加。
 *
 * Revision 1.9  1995/08/26  02:15:23  night
 * RCS の Log マクロの追加。
 *
 *
 */

#include <core.h>
#include <string.h>
#include <vm.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include "func.h"
#include "ready.h"
#include "setting.h"
#include "sync.h"
#include "thread.h"
#include "mpu/interrupt.h"
#include "mpu/mpu.h"
#include "mpu/mpufunc.h"

/***************************************************************************
 *	タスク管理用の変数
 *
 *
 */

static inline thread_t *getTaskParent(const list_t *p);

static inline thread_t *getTaskParent(const list_t *p) {
	return (thread_t*)((ptr_t)p - offsetof(thread_t, queue));
}

/* thread_switch --- タスク切り換え
 *
 * 返値：	エラー番号
 *
 * 処理：	ready_task の中で、一番優先順位の高いタスクをカレント
 *		タスクにする。
 *		実際のタスク切り換えは、context_switch () によっておこなう
 *		そのため、この関数の中での処理は、run_tsk 変数と
 *		ready_task[] の更新を行うのが主となる。
 *		
 */
ER thread_switch(void)
{
    thread_t *next;
    list_t *q;

    enter_critical();
#ifdef TSKSW_DEBUG
    printk("thread_switch(): start\n");
#endif

    if (on_interrupt) {
	delayed_dispatch = TRUE;
	leave_critical();
	return (E_OK);
    }

    if (!dispatchable) {
	leave_critical();
	return (E_CTX);
    }

    q = ready_dequeue();

    if (!q) {
	printk("thread_switch(): error = E_NOEXS\n");	/* */
	leave_critical();
	return (E_NOEXS);
    }

    next = getTaskParent(q);

    /* 選択したタスクが、現タスクならば、何もしないで戻る */
    if (running == next) {
	leave_critical();
	return (E_OK);
    }

    else if (!list_is_empty(&(running->queue))) {
	running->status = TTS_RDY;
    }

    /* 選択したタスクを running にする */
#ifdef TSKSW_DEBUG
    printk("thread_switch: current(%d) -> next(%d)\n",
	    running->id, next->id);
#endif
    if (next->status != TTS_RDY) {
	printk("thread_switch: panic next(%d) stat=%d, eip=%x\n",
		thread_id(next), next->status, next->attr.entry);
	panic("scheduler");
    }

    if (running->mpu.use_fpu)
	fpu_save(running);

    context_prev_sp = &(MPU_KERNEL_SP(running));
    context_next_sp = &(MPU_KERNEL_SP(next));
    running = next;
    running->status = TTS_RUN;

    delayed_dispatch = FALSE;

    context_set_kernel_sp(next->attr.kstack_tail);
    context_switch_page_table(next);
    context_switch();

    /* 正常に終了した：次のタスクスイッチの時にここに戻る */
    if (running->mpu.use_fpu)
	fpu_restore(running);

    return (E_OK);
}
