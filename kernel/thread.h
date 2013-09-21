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
#include <set/tree.h>
#include <core.h>
#include "config.h"
#include "region.h"
#include "wait.h"
#include "mpu/mpu.h"

/* t_tcb --- タスク構造体
 *
 *	この構造体には、タスクの固有情報が含まれる。
 *	ただし、ITRON レベルなので、ファイルの情報などは含まれていない。
 *
 */
typedef struct t_tcb {
    node_t node;
    list_t ready;

    W tskid;			/* タスク ID                    */
    W domain_id;
    W tsklevel;			/* タスクの優先順位             */
    W tsklevel0;		/* タスクの優先順位(初期化時の値) */
    /* 終了したタスクが再度起動するときにはこの値が使われる。     */
    H tskstat;			/* タスクの状態                 */

    /* タスクの待ち状態用の要素 */
    wait_reason_t wait;

/* スタック情報 */
    W stksz;			/* タスクの持つ現在のスタックのサイズ */
    B *stackptr;		/* 現在のスタック領域へのポインタ */

/* カーネル領域のスタック情報                   */
    W stksz0;			/* タスクの持つスタックのサイズ */
    B *stackptr0;		/* カーネルスタックの領域へのポインタ */
    VP stacktop0;

/* タスクの統計情報 */
    W total;			/* 総実行時間                   */
    H quantum;			/* ユーザータスクの連続実行可能時間 ≧ 0 */

    T_MPU_CONTEXT mpu;

/* 仮想記憶情報 */
    T_REGION regions[MAX_REGION];	/* タスクに結びついて */
    /* いる REGION のテーブル */

     W(*page_fault_handler) (W, W);	/* ページフォルト時の処理用関数 */

    UW initial_stack;
} T_TCB;


extern T_TCB *run_task;		/* 現在、走行中のタスク */
extern ID kernel_task_id;

#endif				/* __CORE_TASK_H__ */
