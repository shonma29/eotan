/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* eventflag.h
 *
 *
 */

#ifndef __CORE_EVENTFLAG_H__
#define __CORE_EVENTFLAG_H__	1


/***************************************************************************
 *	イベントフラグ情報
 *
 */
typedef struct eventflag_t
{
  ID		id;			/* イベントフラグ ID 			*/
  ATR		flgatr;			/* イベントフラグ属性			*/
  UINT		iflgptn;		/* イベントフラグの値			*/
  T_TCB		*wait_task;		/* イベントフラグを待っているタスク	*/
  T_TCB		*wait_tail;		/* タスクキューの最後	*/

  VP		exinf;			/* 拡張情報				*/
} T_EVENTFLAG;


#endif /* __CORE_EVENTFLAG_H__ */
