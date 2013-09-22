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

#ifndef _CORE_CONFIG_H_
#define _CORE_CONFIG_H_	1

/* タスク管理関係 */
#define CLOCK		10		/* clock 周期 10 ms */
#define TICKS		(1000/CLOCK)	/* 1 秒間の tick 数 */
#define QUANTUM		(40/CLOCK)	/* 40 ms */

/* メモリ管理関係 */
#define MIN_MEMORY_SIZE		(8 * 1024 * 1024) /* メモリは最小 8 M */
#define MAX_MEMORY_SIZE		(2048UL * 1024UL * 1024UL) /* メモリは最大 2 G */

/* インターバルタイマ */
#define MAX_TIMER	(500)

#endif				/* _CORE_CONFIG_H_ */
