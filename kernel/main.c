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

/* 外部変数の宣言 */
extern W do_timer;

/* 強制終了するタスクのテーブル */
#define TRMTBL_SIZE 10
static int trmtbl_num = 0;
static int trmtbl_top = 0;
static struct TRMTBL {
  ID type; /* POSIX = 0, BTRON = 1 */
  ID tskid; /* task id */
  ID id; /* pid/??? */
} trmtbl[TRMTBL_SIZE];

ER add_trmtbl(ID type, ID tskid, ID id)
{
  int pos;
  if (trmtbl_num == TRMTBL_SIZE) {
    return(E_NOMEM);
  }
  pos = (trmtbl_top+trmtbl_num) % TRMTBL_SIZE;
  trmtbl[pos].type = type;
  trmtbl[pos].tskid = tskid;
  trmtbl[pos].id = id;
  ++trmtbl_num;
  return(E_OK);
}

static ER pick_trmtbl(ID *type, ID *tskid, ID *id)
{
  if (trmtbl_num == 0) {
    return(E_NOMEM);
  }
  *type = trmtbl[trmtbl_top].type;
  *tskid = trmtbl[trmtbl_top].tskid;
  *id = trmtbl[trmtbl_top].id;
  return(E_OK);
}

static ER rm_trmtbl()
{
  if (trmtbl_num == 0) {
    return(E_NOMEM);
  }
  trmtbl_top = (trmtbl_top+1) % TRMTBL_SIZE;
  --trmtbl_num;
  return(E_OK);
}

/*******************************************************************
 * main --- メイン関数
 *
 */
int main(void)
{
    ID type, tskid, id;
    ER errno = E_OK;
#ifdef HALT_WHEN_IDLE
    int do_halt;
#endif

    if (initialize() != E_OK) {
	printk("main: cannot initialize.\n");
	panic("initialize");
    }

    for (;;) {			/* Idle タスクとなる。 */
#ifdef HALT_WHEN_IDLE
        do_halt = 1;
#endif

	if (do_timer) {
	    /* timer に定義されている関数の実行 */
	    check_timer();
	    do_timer = 0;
#ifdef HALT_WHEN_IDLE
	    do_halt = 0;
#endif
	    thread_change_priority(KERNEL_TASK, MAX_PRIORITY);
	}

	/* タスクの強制終了処理 */
	if (trmtbl_num != 0) {
#ifdef HALT_WHEN_IDLE
	  do_halt = 0;
#endif
	  pick_trmtbl(&type, &tskid, &id);
	  switch(type) {
	  case 0: /* POSIX */
	    errno = posix_kill_proc(id);
	    break;
	  case 1: /* BTRON */
	    break;
	  }
	  if (errno == E_OK) {
	    rm_trmtbl();
	    if (trmtbl_num == 0) {
	      /* 強制終了するタスクが無くなったので，優先度を最低に */
	      thread_change_priority(KERNEL_TASK, MAX_PRIORITY);
	    }
	  }
	}

#if defined(HALT_WHEN_IDLE)
	if (do_halt) halt();
#endif

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
    thread_initialize1();

    timer_initialize();		/* インターバルタイマ機能の初期化 */
    time_initialize(rtc_get_time());		/* 時間管理機能の初期化 */
    pit_initialize(TICKS);		/* インターバルタイマの起動       */
    init_interrupt();

    do_timer = 0;

#ifdef AUTO_START
    run_init_program();
#endif

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

