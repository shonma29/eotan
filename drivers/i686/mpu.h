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

#define MPU_KERNEL_SP(taskp) ((taskp)->mpu.esp0)
#define MPU_PAGE_TABLE(taskp) ((taskp)->mpu.cr3)

#endif /* _MPU_MPU_H_ */
