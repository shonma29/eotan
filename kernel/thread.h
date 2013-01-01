/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 *
 */

#ifndef __CORE_TASK_H__
#define __CORE_TASK_H__	1

#include <set/list.h>
#include "core.h"
#include "wait.h"
#include "mpu/fpu.h"

#define KERNEL_TASK	(1)

/* task_spec_interrupt - タスク固有割り込み処理関数
 *
 *	この構造体には、タスク固有の割り込み処理関数を登録する。
 *	(LOWLIB のソフトウェア割り込み登録に使用する)
 */
typedef struct task_spec_interrupt {
    W intr_no;
     ER(*intr_func) (W, VP);
} T_TSI;


/* t_tcb --- タスク構造体
 *
 *	この構造体には、タスクの固有情報が含まれる。
 *	ただし、ITRON レベルなので、ファイルの情報などは含まれていない。
 *
 */
typedef struct t_tcb {
    list_t ready;

    W tskid;			/* タスク ID                    */
    W tsklevel;			/* タスクの優先順位             */
    W tsklevel0;		/* タスクの優先順位(初期化時の値) */
    /* 終了したタスクが再度起動するときにはこの値が使われる。     */
    ATR tskatr;			/* タスク属性                   */
    H tskstat;			/* タスクの状態                 */

    /* タスクの待ち状態用の要素 */
    wait_reason_t wait;

/* スタック情報 */
    W stksz;			/* タスクの持つ現在のスタックのサイズ */
    B *stackptr;		/* 現在のスタック領域へのポインタ */

/* カーネル領域のスタック情報                   */
    W stksz0;			/* タスクの持つスタックのサイズ */
    B *stackptr0;		/* カーネルスタックの領域へのポインタ */

/* タスクの統計情報 */
    W total;			/* 総実行時間                   */
    H quantum;			/* ユーザータスクの連続実行可能時間 ≧ 0 */

#ifdef I386
    T_I386_CONTEXT context;	/* コンテキスト情報 (CPU依存)   */
    UW tss_selector;		/* タスクのセレクタ (CPU依存)   */
    H use_fpu;
    FPU_CONTEXT fpu_context;	/* FPU のコンテキスト情報 */
#endif

/* 仮想記憶情報 */
    T_REGION regions[MAX_REGION];	/* タスクに結びついて */
    /* いる REGION のテーブル */


    T_TSI interrupt[MAX_MODULE];	/* タスク固有の割り込 */
    /* み処理関数テーブル */
    W n_interrupt;

     W(*page_fault_handler) (W, W);	/* ページフォルト時の処理用関数 */

    UW initial_stack;
} T_TCB;


extern T_TCB *run_task;		/* 現在、走行中のタスク */

#define GET_TSKWAIT(tcb)	(tcb.tskwait)


#endif				/* __CORE_TASK_H__ */
