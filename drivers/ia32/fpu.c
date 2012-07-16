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
#include "func.h"
#include "task.h"
#include "sync.h"
#include "../../include/mpu/io.h"

void fpu_save(T_TCB *taskp)
{
  __asm__("fsave %0" : "=m" (taskp->fpu_context));
}

void fpu_restore(T_TCB *taskp)
{
  __asm__("frstor %0" : "=m" (taskp->fpu_context));
}

void fpu_start(T_TCB *taskp)
{
  if (taskp->use_fpu) return;
  enter_critical();
  if (run_task == taskp) {
    __asm__("finit");
    __asm__("fsave %0" : "=m" (taskp->fpu_context));
    taskp->use_fpu = 1;
  }
  else {
    if (run_task->use_fpu) {
      __asm__("fsave %0" : "=m" (run_task->fpu_context));
    }
    __asm__("finit");
    __asm__("fsave %0" : "=m" (taskp->fpu_context));
    if (run_task->use_fpu) {
      __asm__("frstor %0" : "=m" (run_task->fpu_context));
    }
    taskp->use_fpu = 1;
  }
  leave_critical();
}
