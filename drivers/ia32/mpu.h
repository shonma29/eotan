/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* 
  This file is part of BTRON/386

  $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/i386/i386.h,v 1.3 1998/02/25 12:42:53 night Exp $


*/

#ifndef _IA32_MPU_H_
#define _IA32_MPU_H_	1

#include <itron/types.h>
#include "fpu.h"

#define I386	1

#define I386_PAGESIZE	4096

#define MPU_PC(taskp) ((taskp)->mpu.context.eip)
#define MPU_PAGE_TABLE(taskp) ((taskp)->mpu.context.cr3)

/***********************************************************************
 *	directory table entry.
 */
typedef struct
{
  UW	present:1;
  UW	read_write:1;
  UW	u_and_s:1;
  UW	zero2:2;
  UW	access:1;		/* アクセスチェック用ビット */
  UW	dirty:1;		/* 書き込みチェック用ビット */
  UW	zero1:2;
  UW	user:3;
  UW	frame_addr:20;
} I386_DIRECTORY_ENTRY;

/**********************************************************************
 * アドレスマップ・タイプ
 *
 * この型は、80386 のディレクトリテーブルエントリを 1024 並べている。
 * この配列が、仮想メモリマップテーブルとなる。
 *
 */
typedef I386_DIRECTORY_ENTRY	*ADDR_MAP;


/***********************************************************************
 *	page table entry.
 */
typedef struct
{
  UW	present:1;
  UW	read_write:1;
  UW	u_and_s:1;
  UW	zero2:2;
  UW	access:1;
  UW	dirty:1;
  UW	zero1:2;
  UW	user:3;
  UW	frame_addr:20;
} I386_PAGE_ENTRY;

/****************************************************************************
 * T_CONTEXT --- Task State Segment
 *
 */
typedef struct 
{
  UW		backlink;
  UW		esp0;
  UW		ss0;
  UW		esp1;
  UW		ss1;
  UW		esp2;
  UW		ss2;

  UW		cr3;
  UW		eip;
  UW		eflags;
  UW		eax;
  UW		ecx;
  UW		edx;
  UW		ebx;
  UW		esp;
  UW		ebp;
  UW		esi;
  UW		edi;
  UW		es;
  UW		cs;
  UW		ss;
  UW		ds;
  UW		fs;
  UW		gs;
  UW		ldtr;
  UH		t:1;
  UH		zero:15;
  UH		iobitmap;
} T_CONTEXT;

typedef struct {
  T_CONTEXT context;
  UW tss_selector;
  H use_fpu;
  FPU_CONTEXT fpu_context;
} T_MPU_CONTEXT;

/**************************************************************
 *
 *	タスクディスクリプタ
 *
 */
typedef struct
{
  UH	limit0:16;	/*  104 */
  UH	base0:16;	/* タスクベース 0..15		*/
  UH	base1:8;	/* タスクベース 16..23		*/
  UH	type:4;
  UH 	zero0:1;	/* 未使用 (0)			*/
  UH	dpl:2;		/* ディスクリプタ		*/
  UH	present:1;	/* ディスクリプタ present bit	*/
  UH	limit1:4;
  UH	zero1:2;	/* 未使用 (0)			*/
  UH	d:1;
  UH	g:1;		/* 粒度 (1 = 4K バイト単位)	*/
  UH	base2:8;	/* タスクベース 24..31		*/
} TASK_DESC;

#define GET_TSS_ADDR(x)	((x).base0 | (x).base1 << 16 | (x).base2 << 24)
#define GET_TSS_LIMIT(x)	((x).limit0 | (x).limit1 << 16)
#define SET_TSS_ADDR(x,addr)	{ \
   (x).base0 = (UH)((UW)(addr) & 0x00ffff); \
   (x).base1 = (UH)((UW)(addr) >> 16 & 0x00ff); \
   (x).base2 = (UH)((UW)(addr) >> 24 & 0x00ff); }
#define SET_TSS_LIMIT(x,limit)	{\
   (x).limit0 = limit & 0x0000ffff; \
   (x).limit1 = limit >> 16 & 0x0f; }

#define STACK_DIR	SMALL
#define MAXINT		(0x7fffffff)

#define TYPE_TSS	9
#define TSS_BASE	128		/* TSS の GDT 上での位置 */

#define TYPE_GATE	12

#define ACC_KERNEL	0
#define ACC_USER	1

#define EFLAG_IBIT	0x0200
#define EFLAG_IOPL3	0x3000

#define ADDR_MAP_SIZE	1024

#endif /* _IA32_MPU_H_ */
