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

#define MPU_KERNEL_SP(taskp) ((taskp)->mpu.context.esp0)
#define MPU_PAGE_TABLE(taskp) ((taskp)->mpu.context.cr3)

/****************************************************************************
 * T_CONTEXT --- Task State Segment
 *
 */
typedef struct 
{
  VP		esp0;
  VP		cr3;
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

#endif /* _MPU_MPU_H_ */
