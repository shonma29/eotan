/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* flag.h
 *
 *
 */

#ifndef __CORE_FLAG_H__
#define __CORE_FLAG_H__	1

#include <set/list.h>
#include <set/tree.h>

/***************************************************************************
 *	イベントフラグ情報
 *
 */
typedef struct flag_t
{
  node_t node;
  list_t receiver;
  ATR		flgatr;			/* イベントフラグ属性			*/
  UINT		iflgptn;		/* イベントフラグの値			*/
} T_FLAG;


#endif /* __CORE_FLAG_H__ */
