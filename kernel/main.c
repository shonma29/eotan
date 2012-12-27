/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* main.c --- ITRON のメイン関数の定義。
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

#include "core.h"
#include "version.h"
#include "task.h"
#include "func.h"
#include "misc.h"
#include "memory.h"
#include "../include/mpu/io.h"
#ifdef I386
#include "interrupt.h"
#endif
#include "boot.h"
#include "mpu/mpufunc.h"
#include "arch/archfunc.h"

static ER init_itron(void);
static void init_device(void);
void run_init_program(void);
void banner(void);

/* 外部変数の宣言 */
extern W do_timer;
extern UW system_ticks;

/* 強制終了するタスクのテーブル */
#define TRMTBL_SIZE 10
static int trmtbl_num = 0;
static int trmtbl_top = 0;
static struct TRMTBL {
  ID type; /* POSIX = 0, BTRON = 1 */
  ID tskid; /* ITRON task id */
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

ER pick_trmtbl(ID *type, ID *tskid, ID *id)
{
  if (trmtbl_num == 0) {
    return(E_NOMEM);
  }
  *type = trmtbl[trmtbl_top].type;
  *tskid = trmtbl[trmtbl_top].tskid;
  *id = trmtbl[trmtbl_top].id;
  return(E_OK);
}

ER rm_trmtbl()
{
  if (trmtbl_num == 0) {
    return(E_NOMEM);
  }
  trmtbl_top = (trmtbl_top+1) % TRMTBL_SIZE;
  --trmtbl_num;
  return(E_OK);
}

/*******************************************************************
 * itron --- メイン関数
 *
 */
ER itron(void)
{
    ID type, tskid, id;
    ER errno = E_OK;
#ifdef HALT_WHEN_IDLE
    int do_halt;
#endif

    if (init_itron() != E_OK) {
	falldown("main: cannot initialize.\n");
    }

    init_device();

    /* TRMTBL の初期化 */
    trmtbl_num = 0;
    trmtbl_top = 0;

    do_timer = 0;
    system_ticks = 0;

#ifdef AUTO_START
    run_init_program();
#endif

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
	    chg_pri(KERNEL_TASK, MAX_PRIORITY);
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
	      chg_pri(KERNEL_TASK, MAX_PRIORITY);
	    }
	  }
	}

#if defined(HALT_WHEN_IDLE) && defined(I386)
	if (do_halt) asm("hlt");
#endif

	task_switch();
    }
    falldown("falldown.");
/* not return */
    return E_OK;
}


void run(W entry)
{
    W i;
    struct boot_header *info;
    struct module_info *modulep;
    ID rid;
    T_CTSK pktsk;
    T_TCB *new_taskp;

    info = (struct boot_header *) MODULE_TABLE;
    if ((entry < 1) || (entry >= info->count)) {
	printk("module is overflow. (info->count = %d, entry = %d)\n",
	       info->count, entry);
	return;
    }
    modulep = info->modules;
    pktsk.tskatr = TA_HLNG;
    pktsk.itskpri = KERNEL_LEVEL;
    pktsk.stksz = KERNEL_STACK_SIZE;
    pktsk.addrmap = NULL;
    pktsk.startaddr = (FP) modulep[entry].entry;
    if (new_task(&pktsk, &rid, FALSE) != E_OK) {
	printk("Can not make new task.\n");
	return;
    }
#ifdef DEBUG
    printk("Task id = %d, eip = 0x%x\n", rid, modulep[entry].entry);
#endif
    new_taskp = get_tskp(rid);
    if (new_taskp == NULL) {
	printk("new task is NULL.\n");
	return;
    }

    /* 生成したタスクの仮想メモリにモジュールをマッピング */
    /* ただしドライバの場合には、マッピングしない */
    if ((modulep[entry].type == driver) || (modulep[entry].type == lowlib)) {
#ifdef DEBUG
	printk("This module is driver or lowlib. not mapped\n");
#endif
    } else {
	for (i = 0;
	     i < PAGES(modulep[entry].mem_length);
	     i++) {
	    if (vmap
		(new_taskp, modulep[entry].vaddr + i * PAGE_SIZE,
		 modulep[entry].paddr + i * PAGE_SIZE,
		 ACC_USER) == FALSE) {
		printk
		    ("Cannot memory map: virtual addr: 0x%x, phisical addr = 0x%x\n",
		     modulep[entry].vaddr + i * PAGE_SIZE,
		     modulep[entry].paddr + i * PAGE_SIZE);
	    }
	}
#ifdef I386
	if (modulep[entry].type == user){
	  /* 固有のスタックを用意 */
	  make_local_stack(new_taskp, KERNEL_STACK_SIZE, ACC_USER);
	  new_taskp->context.esp = new_taskp->initial_stack;
	  new_taskp->context.ebp = new_taskp->initial_stack;

	  /* セレクタの設定 */
	  new_taskp->context.cs = USER_CSEG | USER_DPL;
	  new_taskp->context.ds = USER_DSEG;
	  new_taskp->context.es = USER_DSEG;
	  new_taskp->context.fs = USER_DSEG;
	  new_taskp->context.gs = USER_DSEG;
	  new_taskp->context.ss = USER_SSEG | USER_DPL;
	}
#endif
    }
    sta_tsk(rid, 0);
    task_switch();
}

void run_init_program(void)
{
    struct boot_header *info;
    W i;

    info = (struct boot_header *) MODULE_TABLE;
    for (i = 1; i < info->count; i++) {
	if (info->modules[i].type == lowlib) {
	    ER errno;

	    errno = init_lowlib(&info->modules[i]);
	    if (errno) {
		printk("cannot initialize LOWLIB.\n");
	    }
	} else {
	    run(i);
	}
    }
}



/* init_itron --- ITRON の初期化を行う。
 *
 */
static ER init_itron(void)
{
    struct boot_header *info; 

    init_interrupt();
    simple_init_console();	/* コンソールに文字を出力できるようにする */
#ifdef DEBUG
    printk("init_itron: start\n");
#endif
    pmem_init();		/* 物理メモリ管理機能の初期化           */
    adjust_vm(physmem_max);
    banner();			/* 立ち上げメッセージ出力               */

    init_kalloc();		/* バイト単位のメモリ管理機能の初期化   */
    queue_initialize();		/* メッセージ管理機能の初期化           */
    init_eventflag();		/* イベントフラグ管理機能の初期化       */
    port_initialize();
    init_task();		/* タスク管理機能の初期化 */

    /* 1番目のタスクを初期化する。そしてそのタスクを以後の処
     * 理で使用する。
     */
    init_task1();

    init_timer();		/* インターバルタイマ機能の初期化 */
    info = (struct boot_header *) MODULE_TABLE;
    init_time(info->machine.clock);		/* 時間管理機能の初期化 */
    start_interval();		/* インターバルタイマの起動       */

/*  init_io (); */
    return (E_OK);
}

static void init_device(void)
{
    W i;

#ifdef DEBUG
    printk("init_device: start.\n");
#endif
    for (i = 0; devices[i] != NULL; i++) {
#ifdef DEBUG
	printk("Init device: 0x%x call.\n", (*devices[i]));
#endif
	(*devices[i]) ();
    }
#ifdef DEBUG
    printk("init_device: end.\n");
#endif
}


/*
 *
 */
void banner(void)
{
    printk("kernel %s for %s/%s\n",
	KERN_VERSION, KERN_ARCH, KERN_MPU);
    printk("base memory = %d Kbytes\n", base_mem / 1024);
    printk("extend memory = %d Kbytes\n", ext_mem / 1024);
}

