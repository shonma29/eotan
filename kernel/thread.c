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
 *
 * 外部関数リスト
 *
 *	init_task ()		--- タスク管理データの初期化
 *	init_task1 ()		--- システム管理タスクの生成と起動
 *	cre_tsk ()		--- タスクの生成
 *	new_task ()		--- 任意のタスク ID でのタスク生成
 *
 * 大域変数
 *	run_task		--- 現在走行中のタスク
 *
 * 内部関数 (static)
 *
 *	make_task_context ()	--- タスク情報を作成する。
 *
 * 内部変数 (static)
 *
 *	task
 *	task_buffer
 *	ready_task
 *	dispatch_flag		ディスパッチするかどうかのフラグ
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
#include <mpu/config.h>
#include "func.h"
#include "misc.h"
#include "ready.h"
#include "sync.h"
#include "thread.h"
#include "mpu/interrupt.h"
#include "mpu/mpufunc.h"

/***************************************************************************
 *	タスク管理用の変数
 *
 *
 */

T_TCB *run_task;		/* 現在、走行中のタスク */

static T_TCB *task;
static T_TCB task_buffer[MAX_TSKID - MIN_TSKID + 1];

static T_TCB *getTaskParent(const list_t *p);

static T_TCB *getTaskParent(const list_t *p) {
	return (T_TCB*)((ptr_t)p - offsetof(T_TCB, ready));
}

/* thread_initialize --- タスク管理の初期化
 *
 * TCB テーブルの内容を初期化する。そして、カレントタスクをタスク番号 -1 
 * のタスクとする。
 *
 */
void thread_initialize(void)
{
    W i;

    /* TCB テーブルの作成と GDT への登録。
     */
    for (i = 0; i < NTASK; i++) {
	task_buffer[i].tskstat = TTS_NON;
	list_initialize(&(task_buffer[i].ready));
    }

    ready_initialize();

    task = &task_buffer[-1];
}


/* thread_initialize1 --- タスク1 の情報を初期化する。
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	タスク 1 の情報を初期化する。
 *		タスク 1 の情報は、proc_table[0] に収められる。
 *		以下の情報を初期化する。
 *		  cr3		現在の cr3 の内容を入れる。
 *		  state		プロセス状態を TTS_RUN に設定(このタスクは実行中)
 *				優先度を 0 に設定
 *				親タスクのタスク id を 0 に設定。
 *		タスクレジスタの値を task[1] の context のアドレス
 *		に設定。
 *		タスクレジスタの値は、設定時には、なにも影響がないが、
 *		タスクを切り換えたとき、現在のタスクレジスタの指すTSS
 *		領域に今現在のタスク情報を退避する。
 *		そのため、事前にタスクレジスタの値を設定しておく必要がある。
 */
void thread_initialize1(void)
{
    /* タスク 1 の情報を初期化する。 */
    memset(&task[KERNEL_TASK], 0, sizeof(T_TCB));	/* zero clear */
    /* タスク状態：走行状態にセット */
    task[KERNEL_TASK].tskstat = TTS_RUN;
    /* タスクレベルは、31(最低位)にセット */
    task[KERNEL_TASK].tsklevel = MAX_PRIORITY;
    /* タスク ID は、KERNEL_TASK(1)にセット */
    task[KERNEL_TASK].tskid = KERNEL_TASK;
    list_initialize(&(task[KERNEL_TASK].wait.waiting));

    set_thread1_context(&(task[KERNEL_TASK]));

    /* 現タスクはタスク1である。 */
    run_task = &(task[KERNEL_TASK]);
    ready_enqueue(run_task->tsklevel, &(run_task->ready));

    set_thread1_start(&(task[KERNEL_TASK]));
}


/* thread_switch --- タスク切り換え
 *
 * 返値：	エラー番号
 *
 * 処理：	ready_task の中で、一番優先順位の高いタスクをカレント
 *		タスクにする。
 *		実際のタスク切り換えは、resume () によっておこなう
 *		そのため、この関数の中での処理は、run_tsk 変数と
 *		ready_task[] の更新を行うのが主となる。
 *		
 */
ER thread_switch(void)
{
    T_TCB *next;
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
    if (run_task == next) {
	leave_critical();
	return (E_OK);
    }

    else if (!list_is_empty(&(run_task->ready))) {
	run_task->tskstat = TTS_RDY;
    }

    /* 選択したタスクを run_task にする */
#ifdef TSKSW_DEBUG
    printk("thread_switch: current(%d) -> next(%d)\n",
	    run_task->tskid, next->tskid);
#endif
    if ((next->tskstat != TTS_RDY)
	    || (MPU_PC(next) == 0)) {
	printk("thread_switch: panic next(%d) stat=%d, eip=%x\n",
		next->tskid, next->tskstat, MPU_PC(next));
	falldown();
    }

    if (run_task->mpu.use_fpu)
	fpu_save(run_task);

    run_task = next;
    run_task->tskstat = TTS_RUN;

    delayed_dispatch = FALSE;

/* resume を呼び出す。resume の引数は、TSS へのセレクタ */
#ifdef TSKSW_DEBUG
    printk("resume (0x%x)\n", ((next->tskid + TSS_BASE) << 3) & 0xfff8);
#endif
    resume((UW) (next->tskid + TSS_BASE) << 3);

    /* 正常に終了した：次のタスクスイッチの時にここに戻る */
    if (run_task->mpu.use_fpu)
	fpu_restore(run_task);

    return (E_OK);
}

/*****************************************************************************
 *
 *		S Y S T E M   C A L L  
 *				  for  T A S K.
 *
 *
 */

/* thread_create --- create task.
 *
 * タスクを1つ生成する。
 * 生成したタスクは、TTS_DMT 状態となり、sta_tsk()を実行するまでは
 * 実際に動くことはない。
 *
 *
 * 引数: tskid 		生成するタスクのID
 *	 pk_ctsk	生成するタスクの属性情報
 *			tskatr		タスク属性
 *			startaddr	タスク起動アドレス
 *			itskpri		タスク起動時優先度
 *			stksz		スタックサイズ
 *			addrmap		アドレスマップ
 *
 * 返り値：	エラー番号
 *		E_OK	正常終了
 *		E_ID	タスクの ID 番号が不正
 *		E_OBJ	同一のタスクが存在している
 *
 */
ER thread_create(ID tskid, T_CTSK * pk_ctsk)
{
    T_TCB *newtask;
    W i;

/* タスク ID の範囲チェック */
    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return (E_ID);
    }
/* 同一 ID のタスクが存在しているかどうかのチェック */
    if (task[tskid].tskstat != TTS_NON) {
	return (E_OBJ);
    }

    newtask = &task[tskid];
    memset(newtask, 0, sizeof(T_TCB));
/* タスク生成 */

    newtask->tskid = tskid;
    newtask->tskstat = TTS_DMT;
    newtask->tsklevel = pk_ctsk->itskpri;
    newtask->tsklevel0 = pk_ctsk->itskpri;
    list_initialize(&(newtask->wait.waiting));

    if (make_task_context(newtask, pk_ctsk) != E_OK) {
	return (E_NOMEM);
    }

    /* 仮想メモリのマッピングテーブルを引数 pk_ctsk の指定したマップに
     * 変更する。
     * 指定がないときには、カレントプロセスと同じマップとなる。
     */
    if (pk_ctsk->addrmap != NULL) {
	set_page_table(newtask, (UW) (pk_ctsk->addrmap));
    } else {
	set_page_table(newtask,
		VTOR((UW) dup_vmap_table((ADDR_MAP) MPU_PAGE_TABLE(run_task))));
    }

    /* タスクのリージョンテーブルを初期化
     */
    for (i = 0; i < MAX_REGION; i++) {
	newtask->regions[i].permission = 0;
    }
    newtask->regions[0].start_addr = (VP) MIN_KERNEL;
    newtask->regions[0].min_size = KERNEL_SIZE;
    newtask->regions[0].max_size = KERNEL_SIZE;
    newtask->regions[0].permission =
	VPROT_READ | VPROT_WRITE | VPROT_KERNEL;

    return (E_OK);
}

/* thread_destroy --- タスクの削除
 * 
 * 引数tskidで指定したタスクを削除する。
 *
 * 引数：
 *	tskid	削除するタスクの ID
 *
 * 戻り値：
 *	E_OK	正常終了
 */
ER thread_destroy(ID tskid)
{
    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return (E_ID);
    }
    if (task[tskid].tskstat == TTS_NON) {
	return (E_NOEXS);
    } else if (task[tskid].tskstat != TTS_DMT) {
	return (E_OBJ);
    }
    /* マッピングテーブルを解放する */
    release_vmap((ADDR_MAP) MPU_PAGE_TABLE(&(task[tskid])));

    /* kernel 領域の stack を開放する */
    pfree((VP) VTOR((UW) task[tskid].stackptr0), PAGES(task[tskid].stksz0));

    task[tskid].tskstat = TTS_NON;
    return (E_OK);
}

/* thread_start --- タスクの起動
 * 
 * 引数tskidで指定したタスクを起動する。
 * 指定したタスクは、cre_tsk で生成されている必要がある。
 *
 * 引数：
 *	tskid	起動するタスクの ID
 *	stacd	タスク起動コード
 *
 * 戻り値：
 *	E_OK	正常終了
 *
 */
ER thread_start(ID tskid, INT stacd)
{
    register int index;

#ifdef TSKSW_DEBUG
    printk("sta_tsk: start\n");
#endif
    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return (E_ID);
    }
    if (task[tskid].tskstat == TTS_NON) {
	return (E_NOEXS);
    }
    if (task[tskid].tskstat != TTS_DMT) {
	return (E_OBJ);
    }

    if (stacd > 0) {
	set_sp(&(task[tskid]), stacd);
    }
    index = task[tskid].tsklevel;
    task[tskid].tskstat = TTS_RDY;
    task[tskid].wait.sus_cnt = 0;
    task[tskid].total = 0;
    enter_critical();
    ready_enqueue(index, &(task[tskid].ready));
    leave_critical();
#ifdef TSKSW_DEBUG
    printk("sta_tsk: task level = %d\n", index);
#endif
    return (E_OK);
}

/******************************************************************************
 * thread_end --- 自タスク終了
 *
 * run_task につながれているタスクを TTS_DMT 状態へ移動する。
 * メモリ資源などは返却しない。
 * 
 * 引数：	なし
 *
 * 返り値：	なし
 *
 * ！		このシステムコールを実行したあとは、もとのコンテキスト
 *		には戻らない。
 *
 * 処理内容：
 *	この関数では、ready タスクキューにつながれているタスクのうち、
 *	最も優先度の高いものを選択し、次に実行するタスクとする。
 *	(選択するタスクがないということはない。--- 必ず走行している 
 *	idle タスクがあるため)
 *	switch_task() では、今走行してるタスクをreadyタスクキューに入
 *	れる。しかし、ext_tsk() では、元のタスクは終了するため、ready
 *	タスクキューには入れず、状態をTTS_DMTにする。
 */
void thread_end(void)
{
    /* 現在のタスクを TTS_DMT 状態にし、選択したタスクを次に走らせるよう */
    /* にする。                                                          */
    enter_critical();
    run_task->tskstat = TTS_DMT;
    list_remove(&(run_task->ready));
    thread_switch();
}

/******************************************************************************
 * thread_end_and_destroy --- 自タスク終了と削除
 *
 * run_task につながれているタスクを TTS_NON 状態へ移動する。
 * メモリ資源などは返却しないが、マッピングされたメモリについては、解
 * 放する。
 * 
 * 引数：	なし
 *
 * 返り値：	なし
 *
 * ！		このシステムコールを実行したあとは、もとのコンテキスト
 *		には戻らない。
 *
 * 処理内容：
 *	この関数では、ready タスクキューにつながれているタスクのうち、
 *	最も優先度の高いものを選択し、次に実行するタスクとする。
 *	(選択するタスクがないということはない。--- 必ず走行している 
 *	idle タスクがあるため)
 *	switch_task() では、今走行してるタスクをreadyタスクキューに入
 *	れる。しかし、ext_tsk() では、元のタスクは終了するため、ready
 *	タスクキューには入れず、状態をTTS_DMTにする。
 */
void thread_end_and_destroy(void)
{
    /* 現在のタスクを TTS_NON 状態にし、選択したタスクを次に走らせるようにする。 */
    /* マッピングテーブルを解放する */
    release_vmap((ADDR_MAP) MPU_PAGE_TABLE(run_task));

    /* kernel 領域の stack を開放する */
    pfree((VP) VTOR((UW) run_task->stackptr0), PAGES(run_task->stksz0));

    enter_critical();
    run_task->tskstat = TTS_NON;
    list_remove(&(run_task->ready));
    thread_switch();
}

/*************************************************************************
 * thread_terminate --- 他タスク強制終了
 *
 * 機能：
 *	引数で指定したタスクを強制的に終了させる。
 *	終了するタスクのもっている資源は解放しない。
 *	ただし、このシステムコールによって終了したタスクは、TTS_DMT 状
 *	態になっただけなので、sta_tsk システムコールによって再開する
 *	ことができる。
 */
ER thread_terminate(ID tskid)
{
    switch (task[tskid].tskstat) {
    case TTS_RUN:		/* 自タスクの場合 */
	if (run_task->tskid == tskid)
	    return (E_OBJ);

	/* ready 状態にあるタスクの場合：強制終了させる */
    case TTS_RDY:
	enter_critical();
	task[tskid].tskstat = TTS_DMT;
	/* レディキューから削除 */
	list_remove(&(task[tskid].ready));
	leave_critical();
	break;

	/* 待ち状態にあるタスクの場合：待ち状態から解放してから強制終了させる。 */
    case TTS_WAI:
	if (task[tskid].wait.type)
	    list_remove(&(task[tskid].wait.waiting));
	enter_critical();
	task[tskid].tskstat = TTS_DMT;
	leave_critical();
	break;
    case TTS_NON:
	return (E_NOEXS);
    default:
	return (E_OBJ);
    }
    return (E_OK);
}

/* thread_change_priority --- プライオリティの変更
 *
 */
ER thread_change_priority(ID tskid, PRI tskpri)
{
    if (tskid == TSK_SELF)
	tskid = run_task->tskid;
    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return (E_ID);
    }
    switch (task[tskid].tskstat) {
    case TTS_NON:
	return (E_NOEXS);

    case TTS_RDY:
    case TTS_RUN:
	enter_critical();
	list_remove(&(task[tskid].ready));
	task[tskid].tsklevel = tskpri;
	ready_enqueue(task[tskid].tsklevel, &(task[tskid].ready));
	leave_critical();
	break;

	break;

    default:
	return (E_OBJ);
    }
    return (E_OK);
}

/* rot_rdq --- 同一プライオリティでのタスクの順序を変更する
 *
 */
ER rot_rdq(PRI tskpri)
{
    if (tskpri == TPRI_RUN)
	tskpri = run_task->tsklevel;
    if ((tskpri < MIN_PRIORITY) || (tskpri > MAX_PRIORITY)) {
	return (E_PAR);
    }

    enter_critical();
    ready_rotate(tskpri);
    leave_critical();

    /* タスクスイッチによる実行権の放棄: 必要無いのかも */
    thread_switch();
    return (E_OK);
}

/*
 * thread_release --- 待ち状態の解除
 */
ER thread_release(ID tskid)
{
    T_TCB *taskp;

    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return (E_ID);
    }

    taskp = &task[tskid];
    switch (taskp->tskstat) {
    case TTS_NON:
	return (E_NOEXS);

    case TTS_WAI:
	enter_critical();
	list_remove(&(task[tskid].wait.waiting));
	taskp->wait.result = E_RLWAI;
	leave_critical();
	release(taskp);
	break;

    default:
	return (E_OBJ);
    }
    return (E_OK);
}

/***********************************************************************************
 * thread_get_id --- 自タスクのタスク ID 参照
 *
 *
 */
ER thread_get_id(ID * p_tskid)
{
    *p_tskid = run_task->tskid;
    return (E_OK);
}

/*****************************************************************************
 * thread_suspend --- 指定したタスクを強制待ち状態に移行
 *
 * 引数：
 *	taskid --- suspend するタスクの ID
 *
 * 返り値：
 *
 *
 * 機能：
 *
 */
ER thread_suspend(ID taskid)
{
    T_TCB *taskp;

    if ((taskid < MIN_TSKID) || (taskid > MAX_TSKID)) {
	return (E_ID);
    }

    if (&task[taskid] == run_task) {
	return (E_OBJ);
    }

    enter_critical();
    taskp = &task[taskid];
    taskp->wait.sus_cnt++;
    switch (taskp->tskstat) {
    case TTS_RDY:
	list_remove(&(taskp->ready));
	taskp->tskstat = TTS_SUS;
	break;

    case TTS_SUS:
	if (taskp->wait.sus_cnt > MAX_SUSPEND_NEST) {
	    taskp->wait.sus_cnt = MAX_SUSPEND_NEST;
	    leave_critical();
	    return (E_QOVR);
	}
	break;

    case TTS_WAI:
	taskp->tskstat = TTS_WAS;
	break;

    case TTS_NON:
	leave_critical();
	return (E_NOEXS);
	/* DO NOT REACHED */

    default:
	leave_critical();
	return (E_OBJ);
	/* DO NOT REACHED */
    }
    leave_critical();
    return (E_OK);
}

/******************************************************************************************
 * thread_resume --- 強制待ち状態のタスクから待ち状態を解除
 *
 * 引数：
 *	taskid --- suspend しているタスクの ID
 *
 *
 * 返り値：
 *	次のエラー番号が返る
 *	
 * E_OK     システムコールは正常に終了した
 * E_ID     タスク ID が不正
 * E_NOEXS  タスクが存在しない(TTS_NON 状態)
 * E_OBJ    タスクの状態が不正(TTS_SUS, TTS_WAS, TTS_NON 以外)
 *
 *
 * 機能：
 *	待ち状態にあるタスクを待ち状態から強制的に解除する。
 *
 *	待ち状態は多重になることがあるが、このシステムコールは、ひとつだけ待ち
 *	を解除する。
 */
ER thread_resume(ID taskid)
{
    T_TCB *taskp;

    if ((taskid < MIN_TSKID) || (taskid > MAX_TSKID)) {
	return (E_ID);
    }

    enter_critical();
    taskp = &task[taskid];
    switch (taskp->tskstat) {
    case TTS_SUS:
	taskp->wait.sus_cnt--;
	if (taskp->wait.sus_cnt <= 0) {
	    taskp->tskstat = TTS_RDY;
	    ready_push(taskp->tsklevel, &(taskp->ready));
	}
	break;

    case TTS_WAS:
	taskp->wait.sus_cnt--;
	if (taskp->wait.sus_cnt <= 0) {
	    taskp->tskstat = TTS_WAI;
	}
	break;

    case TTS_NON:
	leave_critical();
	return (E_NOEXS);

    default:
	leave_critical();
	return (E_OBJ);
    }
    leave_critical();
    return (E_OK);
}

/***********************************************************************
 * new_task --- 任意のタスク ID でのタスク生成
 *
 * 引数：
 *	pk_ctsk	生成するタスクの属性情報
 *		tskatr		タスク属性
 *		startaddr	タスク起動アドレス
 *		itskpri		タスク起動時優先度
 *		stksz		スタックサイズ
 *		addrmap		アドレスマップ
 *	rid	生成したタスクの ID (返り値)
 *
 * 返り値：
 *	エラー番号
 *	E_OK	正常終了
 *
 * 機能：
 *	new_task は、新しいタスクを作成するという cre_tsk とほとんど
 *	同じ機能をもつ。ただし、cre_tsk がタスク ID を必要とするのに対
 *	し、new_task は、タスク ID を自動的に割りあてる。
 *
 */
ER new_task(T_CTSK * pk_ctsk, ID * rid)
{
    ID i;
    ER err;

    for (i = MIN_USERTASKID; i <= MAX_USERTASKID; i++) {
	err = thread_create(i, pk_ctsk);
	if (err == E_OK) {
	    *rid = i;
	    return (E_OK);
	}
    }
    return (E_NOMEM);
}


T_TCB *get_thread_ptr(ID tskid)
{
    if ((tskid < MIN_TSKID) || (tskid > MAX_TSKID)) {
	return ((T_TCB *) NULL);
    }
    return (&task[tskid]);
}
