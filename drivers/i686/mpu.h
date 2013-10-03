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

#ifndef _MPU_MPU_H_
#define _MPU_MPU_H_	1

#include <core.h>
#include "fpu.h"

#define I386	1

#define MPU_KERNEL_SP(taskp) ((taskp)->mpu.context.esp0)
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
  UW		esp;
  UW		esp0;
  UW		cr3;
} T_CONTEXT;

typedef struct {
  T_CONTEXT context;
  H use_fpu;
  FPU_CONTEXT fpu_context;
} T_MPU_CONTEXT;

#define ACC_KERNEL	0
#define ACC_USER	1

#define EFLAG_IBIT	0x0200
#define EFLAG_IOPL3	0x3000

#define ADDR_MAP_SIZE	1024

#endif /* _MPU_MPU_H_ */
