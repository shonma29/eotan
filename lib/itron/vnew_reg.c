/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_virtual.c,v 1.1 1999/04/18 17:48:34 monaka Exp $ */

/*
 * $Log: sys_virtual.c,v $
 * Revision 1.1  1999/04/18 17:48:34  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.8  1998/02/25 12:33:35  night
 * vmap_reg () の引数の数がひとつ増えたことによる変更。
 *
 * Revision 1.7  1998/02/16 14:14:18  night
 * vget_phs() 関数の追加。
 *
 * Revision 1.6  1997/10/24 14:04:24  night
 * vdel_reg() の追加。
 *
 * Revision 1.5  1997/10/23 14:31:06  night
 * vunm_reg () の追加
 *
 * Revision 1.4  1996/11/20 12:08:29  night
 * vput_reg() の追加。
 *
 * Revision 1.3  1996/11/07  12:40:43  night
 * 関数 vget_reg () の追加。
 *
 * Revision 1.2  1996/07/23  17:17:08  night
 * IBM PC 用の make 環境に merge
 *
 * Revision 1.1  1996/07/22  23:52:06  night
 * 最初の登録
 *
 *
 */

/*
 *	仮想メモリ関連
 *
 */


#include <mpu/call_kernel.h>
#include <core.h>
#include <api.h>

ER
vnew_reg (ID	id, 	/* task ID */
	  VP	start,	/* リージョンの開始アドレス */
	  W	min,	/* リージョンの最小(初期)サイズ */
	  W	max,	/* リージョンの最大サイズ */
	  UW	perm,	/* リージョンのパーミッション */
	  FP	handle,	/* リージョン内でページフォールトが発生したと */
			/* きの処理の指定 */
	  ID	*rid)    /* region number */
{
  int i;
  ER err;
  
  for (i = 0; i < MAX_REGION; ++i) {
    err = vcre_reg(id, i, start, min, max, perm, handle);
    if (err == E_OK) {
      *rid = i;
      return(E_OK);
    }
  }
  return(E_NOMEM);
}
