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

#include "../../kernel/core.h"
#include "../../kernel/api.h"

/*
 * システムクロック設定
 */
ER set_tim(SYSTIME *pk_tim)
{
  return call_syscall (SYS_SET_TIM, pk_tim);
}

/*
 * システムクロック参照
 */
ER get_tim(SYSTIME *pk_tim)
{
  return call_syscall (SYS_GET_TIM, pk_tim);
}

/*
 * タスク遅延
 */

ER dly_tsk(DLYTIME dlytim)
{
  return call_syscall (SYS_DLY_TSK, dlytim);
}

/*
 * アラームハンドラ定義
 */

ER def_alm(HNO almo, T_DALM *pk_dalm)
{
  return call_syscall (SYS_DEF_ALM, almo, pk_dalm);
}