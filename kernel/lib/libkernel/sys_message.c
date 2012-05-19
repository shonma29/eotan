/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_message.c,v 1.1 1999/04/18 17:48:34 monaka Exp $ */
static char	rcsid[] = "$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_message.c,v 1.1 1999/04/18 17:48:34 monaka Exp $";


/*
 *	メッセージ関連システムコールを呼ぶためのライブラリ
 *
 */


#include "../../core/types.h"
#include "../../core/itron.h"
#include "../../core/api.h"
#include "../../core/errno.h"

/*
 * メッセージバッファの作成
 */
ER
cre_mbf (ID id, T_CMBF *pk_cmbf)
{
  return (call_syscall (SYS_CRE_MBF, id, pk_cmbf));
}


/* 
 *  メッセージバッファの削除
 */
ER
del_mbf (ID id)
{
  return (call_syscall (SYS_DEL_MBF, id));
}


/*
 *  メッセージの送信
 */
ER
snd_mbf (ID id, INT size, VP msg)
{
  return (call_syscall (SYS_SND_MBF, id, size, msg));
}


/* 
 *  メッセージの送信 (ポーリング機能つき)
 */
ER
psnd_mbf (ID id, INT size, VP msg)
{
  return (call_syscall (SYS_PSND_MBF, id, size, msg));
}


/*
 *  メッセージの送信 (タイムアウト機能つき)
 */
ER
tsnd_mbf (ID id, INT size, VP msg, TMO tmout)
{
  return (call_syscall (SYS_TSND_MBF, id, size, msg, tmout));
}


/*
 *  メッセージの受信
 */
ER
rcv_mbf (VP msg, INT *size, ID id)
{
  return (call_syscall (SYS_RCV_MBF, msg, size, id));
}


/*
 *  メッセージの受信 (ポーリング機能つき)
 */

ER
prcv_mbf (VP msg, INT *size, ID id)
{
  return (call_syscall (SYS_PRCV_MBF, msg, size, id));
}


/*
 * メッセージ受信 (タイムアウト機能つき)
 */
ER
trcv_mbf (VP msg, INT *size, ID id, TMO tmout)
{
  return (call_syscall (SYS_TRCV_MBF, msg, size, id, tmout));
}


