/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* memory.h
 *
 */

#ifndef __CORE_MEMORY_H__
#define __CORE_MEMORY_H__	1

#include "config.h"

#define MAX_MEM			(MAX_MEMORY_SIZE)

#define MAX_MEMORY_MAP_SIZE		(MAX_MEM / PAGE_SIZE)


#define  MEM_FREE	(0x00)
#define  MEM_USE	(0x01)
#define  MEM_MAP	(0x02)

extern unsigned int	physmem_max;	/* 物理メモリの最大量 (base_mem + ext_mem) */
extern unsigned int	base_mem;	/* 下位にあるメモリのサイズ  */
					/* (PC9801 の場合 640K) */
extern unsigned int	ext_mem;	/* 拡張メモリのサイズ */


#endif /* __CORE_MEMORY_H__ */

