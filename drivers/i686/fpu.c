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

#include <mpu/io.h>
#include "core.h"
#include "thread.h"
#include "ready.h"
#include "sync.h"

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

void fpu_start(thread_t *taskp)
{
  if (taskp->mpu.use_fpu) return;
  enter_critical();
  if (running == taskp) {
    __asm__("finit");
    __asm__("fsave %0" : "=m" (taskp->mpu.fpu_context));
    taskp->mpu.use_fpu = 1;
  }
  else {
    if (running->mpu.use_fpu) {
      __asm__("fsave %0" : "=m" (running->mpu.fpu_context));
    }
    __asm__("finit");
    __asm__("fsave %0" : "=m" (taskp->mpu.fpu_context));
    if (running->mpu.use_fpu) {
      __asm__("frstor %0" : "=m" (running->mpu.fpu_context));
    }
    taskp->mpu.use_fpu = 1;
  }
  leave_critical();
}
