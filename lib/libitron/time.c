/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* 
 * sys_time.c -- 時間管理関係のシステムコール
 */

#include <itron/types.h>
#include <itron/syscall.h>
#include <mpu/call_kernel.h>
#include <nerve/svcno.h>

/*
 * システムクロック設定
 */
ER set_tim(SYSTIME *pk_tim)
{
  return ncall(SVC_TIME_SET, pk_tim);
}

/*
 * システムクロック参照
 */
ER get_tim(SYSTIME *pk_tim)
{
  return ncall(SVC_TIME_GET, pk_tim);
}

/*
 * タスク遅延
 */

ER dly_tsk(DLYTIME dlytim)
{
  return ncall(SVC_THREAD_DELAY, dlytim);
}

/*
 * アラームハンドラ定義
 */

ER def_alm(HNO almo, T_DALM *pk_dalm)
{
  return ncall(SVC_ALARM_CREATE, almo, pk_dalm);
}
