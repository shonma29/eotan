/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/h/region.h,v 1.4 1999/12/29 17:03:14 monaka Exp $ */

/*
 * $Log: region.h,v $
 * Revision 1.4  1999/12/29 17:03:14  monaka
 * Minor modify. Some numeric defines are braced.
 *
 * Revision 1.3  1999/03/15 00:27:49  monaka
 * Minor fix.
 *
 * Revision 1.2  1998/12/12 05:40:35  monaka
 * Minor fix for comment to avoid warning.
 *
 * Revision 1.1  1996/07/22 13:39:18  night
 * IBM PC 版 ITRON の最初の登録
 *
 * Revision 1.2  1995/09/21  15:51:22  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.1  1995/03/18  14:49:08  night
 * 最初の登録
 *
 *
 */

#ifndef __CORE_REGION_H__
#define __CORE_REGION_H__	1

#include <itron/types.h>

#define MAX_REGION		(4)	/* 各タスクに存在できる REGION の数 */
#define TEXT_REGION		0
#define DATA_REGION		1
#define HEAP_REGION		2
#define STACK_REGION		3

/*
 *
 */
struct region_t
{
  VP	start_addr;
  W	min_size;
  W	max_size;
  UW	permission;
};

typedef struct region_t	T_REGION;


#endif /* __CORE_REGION_H__ */
