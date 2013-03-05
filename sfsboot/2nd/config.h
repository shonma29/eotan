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
#include "../../kernel/config.h"
#include "../../kernel/mpu/mpu.h"

#define MAX_PAGEENT	(MIN_MEMORY_SIZE / I386_PAGESIZE)
#define MAX_DIRENT	(I386_PAGESIZE / sizeof(I386_DIRECTORY_ENTRY))

#define BOOT_PATH	"/system/kern"

#endif /* __CONFIG_H__ */
