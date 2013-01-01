/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_eventflag.c,v 1.2 2000/01/28 08:55:29 monaka Exp $ */

/*
 * $Log: sys_eventflag.c,v $
 * Revision 1.2  2000/01/28 08:55:29  monaka
 * Some define of function-call number was changed.
 *
 * Revision 1.1  1999/04/18 17:48:34  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.2  1996/07/23 17:17:07  night
 * IBM PC 用の make 環境に merge
 *
 * Revision 1.1  1996/07/22  23:52:06  night
 * 最初の登録
 *
 * Revision 1.2  1995/09/21  15:51:43  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.1  1995/09/19  18:07:38  night
 * 最初の登録
 *
 *
 */

/*
 * イベントフラグ関係のシステムコールインタフェース関数
 */

#include <core.h>
#include <api.h>
#include "call_kernel.h"


ER
del_flg (ID flgid)
{
  return ncall(SYS_DEL_FLG, flgid);
}

ER
set_flg (ID flgid, UINT setptn)
{
  return ncall(SYS_SET_FLG, flgid, setptn);
}

ER
clr_flg (ID flgid, UINT clrptn)
{
  return ncall(SYS_CLR_FLG, flgid, clrptn);
}

ER
wai_flg (UINT *flgptn, ID flgid, UINT waiptn, UINT wfmode)
{
  return ncall(SYS_WAI_FLG, flgptn, flgid, waiptn, wfmode);
}
