/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* main.c --- メイン関数の定義。
 *
 *
 */

/*
 * $Log: main.c,v $
 * Revision 1.18  2000/04/03 14:34:40  naniwa
 * to call timer handler in task
 *
 * Revision 1.17  2000/02/06 09:10:54  naniwa
 * minor fix
 *
 * Revision 1.16  1999/11/14 14:53:35  naniwa
 * add time management function
 *
 * Revision 1.15  1999/04/13 04:49:02  monaka
 * Some code in 2nd/config.h is separated to ITRON/h/config_boot.h
 *
 * Revision 1.14  1999/04/13 04:15:04  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.13  1999/04/12 14:49:06  monaka
 * Commented out a unused variable define.
 *
 * Revision 1.12  1999/04/12 13:29:18  monaka
 * printf() is renamed to printk().
 *
 * Revision 1.11  1999/03/16 13:02:49  monaka
 * Modifies for source cleaning. Most of these are for avoid gcc's -Wall message.
 *
 * Revision 1.10  1998/02/25 12:38:42  night
 * vmap() 関数の仕様変更による修正。
 * (仮想ページのアクセス権を指定する引数がひとつ増えた)
 *
 * Revision 1.9  1997/10/11 16:21:40  night
 * こまごました修正
 *
 * Revision 1.8  1997/09/21 13:32:01  night
 * log マクロの追加。
 *
 *
 */

#include <core.h>
#include <fstype.h>
#include <global.h>
#include <major.h>
#include <mpu/config.h>
#include <mpu/io.h>
#include <set/lf_queue.h>
#include "version.h"
#include "thread.h"
#include "func.h"
#include "memory.h"
#include "setting.h"
#include "mpu/interrupt.h"
#include "mpu/mpufunc.h"
#include "arch/archfunc.h"

static ER initialize(void);
static void banner(void);

ID kernel_task_id = 0;

/* 外部変数の宣言 */
extern W do_timer;

/* 強制終了するタスクのテーブル */
#define TRMTBL_SIZE 10
static char kqbuf[lfq_buf_size(sizeof(ID), TRMTBL_SIZE)];
volatile lfq_t kqueue;


/*******************************************************************
 * main --- メイン関数
 *
 */
int main(void)
{
    if (initialize() != E_OK) {
	printk("main: cannot initialize.\n");
	panic("initialize");
    }

    for (;;) {			/* Idle タスクとなる。 */
	ID id;

	if (do_timer) {
	    /* timer に定義されている関数の実行 */
	    check_timer();
	    do_timer = 0;
	}

	/* タスクの強制終了処理 */
	if (lfq_dequeue(&id, &kqueue) == QUEUE_OK) {
	    posix_kill_proc(id);

	} else if (run_task->tsklevel != MAX_PRIORITY) {
	    thread_change_priority(kernel_task_id, MAX_PRIORITY);

	} else
	    halt();

	thread_switch();
    }

/* not return */
    return E_OK;
}

/* initialize --- 初期化を行う。
 *
 */
static ER initialize(void)
{
    system_info_t *sysinfo = (system_info_t *)SYSTEM_INFO_ADDR;

    sysinfo->root.device = get_device_id(DEVICE_MAJOR_ATA, 0);
    sysinfo->root.fstype = FS_SFS;
    sysinfo->initrd.start = 0;
    sysinfo->initrd.size = 0;

    pic_initialize();
    banner();			/* 立ち上げメッセージ出力               */

    paging_reset();
    context_initialize();
    api_initialize();
    fpu_initialize();
#ifdef DEBUG
    printk("initialize: start\n");
#endif
    queue_initialize();
    port_initialize();
    thread_initialize();		/* タスク管理機能の初期化 */
    kcall_initialize();

    /* 1番目のタスクを初期化する。そしてそのタスクを以後の処
     * 理で使用する。
     */
    kernel_task_id = thread_initialize1();
    lfq_initialize(&kqueue, kqbuf, sizeof(ID), TRMTBL_SIZE);

    timer_initialize();		/* インターバルタイマ機能の初期化 */
    time_initialize(rtc_get_time());		/* 時間管理機能の初期化 */
    pit_initialize(TICKS);		/* インターバルタイマの起動       */
    init_interrupt();

    do_timer = 0;

    run_init_program();

    return (E_OK);
}

/*
 *
 */
static void banner(void)
{
    printk("kernel %s for %s/%s\n",
	KERN_VERSION, KERN_ARCH, KERN_MPU);
}

