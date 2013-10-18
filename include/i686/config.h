/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#ifndef _MPU_CONFIG_H_
#define _MPU_CONFIG_H_

#define GDT_ADDR	(0x80001000UL)
#define IDT_ADDR	(0x80001800UL)
#define MAX_GDT		(5)	/* GDT の最大のインデックス */
#define MAX_IDT		(47)	/* IDT の最大のインデックス */

#define PAGE_DIR_ADDR	(0x2000)	/* 仮想ページディレクトリテーブル */

#define KERNEL_STACK_SIZE	(PAGE_SIZE * 1)
#define USER_STACK_SIZE	(PAGE_SIZE * 1)

/* メモリ管理関係 */
#define PAGE_SIZE	4096
#define PAGE_SHIFT	(12)
#define DIR_SHIFT	(22)
#define PAGE_MASK	(0x003FF000L)
#define DIR_MASK	(0xFFC00000UL)
#define OFFSET_MASK	(0x00000FFFL)
#define	MIN_KERNEL	(0x80000000UL)
#define MAX_KERNEL	(0xFFFFFFFFUL)
#define KERNEL_SIZE	(MAX_KERNEL - MIN_KERNEL)

/* マネージャータスクのヒープ領域の推奨値 */
#define VADDR_HEAP	(0x10000000L)
#define STD_HEAP_SIZE	(0x3FFFFFFFL)

#define STD_STACK_SIZE	 (0x3FF00000L)
#define VADDR_STACK_TAIL (0x7FF00000L)
#define VADDR_STACK_HEAD (VADDR_STACK_TAIL - STD_STACK_SIZE)

#endif				/* _CORE_CONFIG_H_ */
