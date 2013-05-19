/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* sys_task.c -- タスク関係のシステムコール
 *
 * $Id: sys_task.c,v 1.4 2000/04/03 14:37:35 naniwa Exp $
 */

#include <itron/types.h>
#include <itron/syscall.h>
#include <mpu/call_kernel.h>
#include <nerve/svcno.h>

/* exd_tsk  --- 自タスク終了と削除
*/
void
exd_tsk (void)
{
  ncall(SVC_THREAD_END_AND_DESTROY);
}

/* get_tid  --- 自タスクのタスク ID 参照
*/
ER
get_tid (ID *rid)
{
  return ncall(SVC_THREAD_GET_ID, rid);
}
