/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#ifndef _MPU_CONFIG_H_
#define _MPU_CONFIG_H_	1

#define GDT_ADDR	(0x80001000UL)
#define IDT_ADDR	(0x80001800UL)
#define MAX_GDT		(255)	/* GDT の最大のインデックス */
#define MAX_IDT		(255)	/* IDT の最大のインデックス */

#define TASK_DESC_MIN	TSS_BASE
#define KERNEL_CSEG	(0x0008)	/* カーネルのコードセグメント */
#define KERNEL_DSEG	(0x0010)	/* カーネルのデータセグメント */
#define USER_CSEG	(0x0018)	/* ユーザのコードセグメント */
#define USER_DSEG	(0x0020)	/* ユーザのデータセグメント */
#define USER_SSEG	(0x0028)	/* ユーザのスタックセグメント */
#define SVC_GATE	(0x0030)	/* システムコール用ゲート */

#define PAGE_DIR_ADDR	(0x2000)	/* 仮想ページディレクトリテーブル */
#define PAGE_ENTRY_ADDR	(0x3000)	/* 仮想ページテーブル */

#define KERNEL_DPL	0
#define USER_DPL	3

#define KERNEL_STACK_SIZE	(PAGE_SIZE * 1)
#define POSIX_STACK_SIZE	(PAGE_SIZE * 1)

/* メモリ管理関係 */
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

/* マネージャータスクのヒープ領域の推奨値 */
#define VADDR_HEAP	(0x10000000L)
#define STD_HEAP_SIZE	(0x3FFFFFFFL)

#define STD_STACK_SIZE	 (0x3FF00000L)
#define VADDR_STACK_TAIL (0x7FF00000L)
#define VADDR_STACK_HEAD (VADDR_STACK_TAIL - STD_STACK_SIZE)

#endif				/* _CORE_CONFIG_H_ */
