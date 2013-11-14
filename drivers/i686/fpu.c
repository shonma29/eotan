/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log$
 *
 */
#include "core.h"
#include "thread.h"

void fpu_initialize(void)
{
  __asm__("finit");
}

void fpu_save(thread_t *taskp)
{
  __asm__("fsave %0" : "=m" (taskp->mpu.fpu_context));
}

void fpu_restore(thread_t *taskp)
{
  __asm__("frstor %0" : "=m" (taskp->mpu.fpu_context));
}
