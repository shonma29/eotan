/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_lowlib.c,v 1.1 1999/04/18 17:48:34 monaka Exp $ */

#include <core.h>
#include <api.h>
#include <lowlib.h>
#include "call_kernel.h"


/* lowlib を特定のタスクに attach する
 */
ER
lod_low (ID task, B *name)
{
  ER erResult;
  struct a
    {
      ID task;
      B *name;
    } args;

  args.task = task;
  args.name = name;

  erResult = call_syscall (SYS_VSYS_MSC, 3, &args);

  return (erResult);
}
