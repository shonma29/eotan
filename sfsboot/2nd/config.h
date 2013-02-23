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

#include "../../kernel/boot.h"

#define MAJOR_VER	0
#define MINOR_VER	2

#define MIN_MEMORY_SIZE		(16 * 1024 * 1024) /* メモリは最小 16 M */

#define MAX_PAGEENT	(4 * 1024) /* 最大 16 M 分のページエントリ */
#define MAX_DIRENT	1024

#define BASE_MEM	(1024 * 1024)	/* for IBM PC/AT */

#define BOOT_PATH	"/system/kern"

#endif /* __CONFIG_H__ */
