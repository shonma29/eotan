/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/**************************************************************************
 *
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__	1

#include "config_boot.h"

#define MAJOR_VER	0
#define MINOR_VER	2

#define MIN_MEMORY_SIZE		(16 * 1024 * 1024) /* メモリは最小 16 M */

#define MAX_PAGEENT	(1024 * (2 + 16)) /* 最大 16 M 分のページエントリ */
#define MAX_DIRENT	1024

#define BASE_MEM	(1024 * 1024)	/* for IBM PC/AT */

#define MALLOC_SIZE	(1024 * 100)	/* 100K バイトを malloc 用に使 */
					/* う */

#define BOOT_PATH	"/system/btron386"

#define TIMER0_WRITE	0x40
#define TIMER0_READ	0x40
#define TIMER_CONTROL	0x43
#define CLOCK		50		/* clock 周期 10 ms */
#define TICKS		(1000/CLOCK)	/* 1 秒間の tick 数 */
#define TIMER_FREQ	1193182L
#define BTRON_OFFSET	473353200L

extern struct file_system	root_fs;
extern struct file		root;

#endif /* __CONFIG_H__ */
