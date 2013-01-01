/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* message.h --- メッセージバッファ管理
 *
 *
 */

#ifndef __CORE_MESSAGE_H__
#define __CORE_MESSAGE_H__		1

#include <set/list.h>

/*******************************************************************************
 * メッセージバッファのヘッダ
 *
 */
typedef struct message_head_t
{
  list_t message;
  ATR			mbfatr;
  int			total_size;	/* 空きバッファのサイズ */
  list_t sender;
  list_t receiver;
  INT			bufsz;		/* バッファの最大長	*/
  INT			msgsz;		/* メッセージの最大長	*/
} T_MSGHEAD;



/*******************************************************************************
 * メッセージエントリ
 *
 *	メッセージはこのパケットに詰められる。
 *
 */
typedef struct message_t
{
  list_t message;
  INT			size;
  VP			buf;
} T_MSG;


#endif /* __CORE_MESSAGE_H__ */
