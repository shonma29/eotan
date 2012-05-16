/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* 
  This file is part of BTRON/386

  $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/h/config.h,v 1.9 2000/07/10 14:22:05 kishida0 Exp $

  

*/

#ifndef _ITRON_CONFIG_H_
#define _ITRON_CONFIG_H_	1

#define CALL_HANDLER_IN_TASK	1 /* タイマーのハンドラーの実行を task 1 で行う */

#include "mpu/i386.h"

#define GDT_ADDR	(0x80001000UL)
#define IDT_ADDR	(0x80002000UL)
#define MAX_GDT		(255)	/* GDT の最大のインデックス */
#define MAX_IDT		(255)	/* IDT の最大のインデックス */

#define TASK_DESC_MIN	TSS_BASE
#define KERNEL_CSEG	(0x0008)	/* カーネルのコードセグメント */
#define KERNEL_DSEG	(0x0010)	/* カーネルのデータセグメント */
#define USER_CSEG	(0x0018)	/* ユーザのコードセグメント */
#define USER_DSEG	(0x0020)	/* ユーザのデータセグメント */
#define USER_SSEG	(0x0028)	/* ユーザのスタックセグメント */
#define ITRON_GATE	(0x0030)	/* ITRON システムコール用ゲート */
#define POSIX_GATE	(0x0038)	/* POSIX システムコール用ゲート */
#define BTRON_GATE	(0x0040)	/* BTRON システムコール用ゲート */

#define PAGE_DIR_ADDR	(0x3000)	/* 仮想ページディレクトリテーブル */
#define PAGE_ENTRY_ADDR	(0x4000)	/* 仮想ページテーブル */

/* タスク管理関係 */
#define MAX_TSKID	(128)	/* 最大のタスク ID 数   */
#define MIN_TSKID	(1)	/* 最小のタスク ID 数   */
#define NTASK		(MAX_TSKID - MIN_TSKID + 1)	/* タスク数     */

#define MIN_PRIORITY	(0)	/* 最小のプライオリティ */
#define MAX_PRIORITY	(31)	/* 最大のプライオリティ */

#define MIN_USERTASKID	(21)
#define MAX_USERTASKID	MAX_TSKID

#define MAX_SUSPEND_NEST	(10)	/* サスペンドのネストのオーバーフロー */

#define KERNEL_DPL	0
#define USER_DPL	3

#define KERNEL_LEVEL	(2)
#define USER_LEVEL	(16)
#define MID_LEVEL	(10)

#define KERNEL_STACK_SIZE	(PAGE_SIZE * 10)
#define POSIX_STACK_SIZE	(PAGE_SIZE * 10)

#define CLOCK		10		/* clock 周期 10 ms */
#define TICKS		(1000/CLOCK)	/* 1 秒間の tick 数 */
#define QUANTUM		(40/CLOCK)	/* 40 ms */
#define HALT_WHEN_IDLE	1	/* IDLE 時に CPU を Halt で停止する */

/* セマフォ管理関係 */
#define MIN_SEMAID		(1)
#define MAX_SEMAID		(256)
#define NSEMAPHORE		(MAX_SEMAID - MIN_SEMAID + 1)
#define MIN_USERSEMAID		(10)
#define MAX_USERSEMAID		MAX_SEMAID


/* イベントフラグ管理関係 */
#define MIN_EVENTFLAG		(1)
#define MAX_EVENTFLAG		(256)
#define NEVENTFLAG		(MAX_SEMAID - MIN_SEMAID + 1)
#define MIN_USEREVENTID		(10)
#define MAX_USEREVENTID		MAX_EVENTFLAG


/* メッセージバッファ管理関係 */
#define MIN_MSGBUF		(1)
#define MAX_MSGBUF		(256)
#define NMSGBUF			(MAX_MSGBUF - MIN_MSGBUF + 1)
#define MAX_MSGENTRY		(1000)
/*
 * 自由に使えるメッセージバッファの最小の ID と最大の ID
 */
#define MIN_USERMBFID		(100)
#define MAX_USERMBFID		MAX_MSGBUF


/* メモリ管理関係 */
#define MAX_MEMPOOL	(100)
#ifndef PAGE_SIZE
#define PAGE_SIZE	I386_PAGESIZE
#endif
#define PAGE_SHIFT	(12)
#define DIR_SHIFT	(22)
#define PAGE_MASK	(0x003FF000L)
#define DIR_MASK	(0xFFC00000UL)
#define OFFSET_MASK	(0x00000FFFL)
#define	MIN_KERNEL	(0x80000000UL)
#define MAX_KERNEL	(0xFFFFFFFFUL)
#define KERNEL_SIZE	(MAX_KERNEL - MIN_KERNEL)

#define MEM_SIZE		(0x100000)	/* 拡張メモリは 1M バイト */
#define MAX_FREEMEM_ENTRY	(1000)	/* フリーメモリを管理するためのテーブル */
#define MAX_MEMORY_POOL		(100)
#define MIN_MEMORY_SIZE		(16 * 1024 * 1024) /* メモリは最小 16 M */
#define MAX_MEMORY_SIZE		(128 * 1024 * 1024) /* メモリは最大 128 M */
#define MEMORY_POOL_BASE	(0x80010000UL)	/* システムメモリとして管理する領域のベース */

#define SYSTEM_MEMORY_POOL	(0)	/* システムメモリプールの ID */

#define MAX_REGION		(5)	/* 各タスクに存在できる REGION の数 */
#define KERNEL_REGION		0
#define TEXT_REGION		1
#define DATA_REGION		2
#define HEAP_REGION		3
#define STACK_REGION		4

/* マネージャータスクのヒープ領域の推奨値 */
#define VADDR_HEAP	(0x10000000L)
#define STD_HEAP_SIZE	(0x3FFFFFFFL)

#define STD_STACK_SIZE	 (0x3FF00000L)
#define VADDR_STACK_TAIL (0x7FF00000L)
#define VADDR_STACK_HEAD (VADDR_STACK_TAIL - STD_STACK_SIZE)

/* IO 管理関係 */
#define MAX_DEVICE_NAME		(16)	/* デバイス名の最大長 (16 バイト) */
#define MIN_DEVID		(0)
#define MAX_DEVID		(19)
#define MAX_DEVICE		(MAX_DEVID - MIN_DEVID + 1)	/* デバイスの最大数 */


/* インターバルタイマ */
#define MAX_TIMER	(500)


/* システムタスクのリスト */
#define NSYSTEM_TASK	(20)	/* システムタスクの数 */

#define ITRON_IDLE	(1)
#define ITRON_RS232C	(2)	/* ITRON 用 RS232C ドライバタスク */
#define ITRON_KEYBOARD	(3)
#define ITRON_CONSOLE	(4)	/* コンソールデバイスドライバ */
#define ITRON_DEBUG	(5)	/* デバッグ用プロセス: コマンドインタプリタ用 */

/*
 * ポートマネージャが要求受けつけ用に指定するメッセージバッファの ID 
 */
#define PORT_MANAGER_PORT	(11)


/* システムセマフォのリスト */
#define ITRON_KEYBOARD_SEM	(3)	/* キーボード入力用のセマフォ    */

/* 時間管理 */
#define MAX_CYCLIC		(50)	/* 周期起動ハンドラの最大数 */
#define MAX_ALARM		(50)	/* アラームハンドラの最大数 */

#define FDC_ALARM		(0)

/* システムメッセージバッファのリスト */
#define ITRON_DEBUG_MBF		(1)	/* デバッグ用プロセス: コマンドインタプリタで使用 */
#define ITRON_RS232C_MBF	(2)	/* ITRON 用 RS232C ドライバタスクで使用 */
#define ITRON_KEYBOARD_MBF	(3)	/* キーボード入力用のメッセージバッファ */


extern W ndevice;
extern ER(*devices[]) (void);


#endif				/* _ITRON_CONFIG_H_ */
