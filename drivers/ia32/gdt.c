/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
** gdt.c --- GDT アクセスルーチン集
**
*/


#include <string.h>
#include "core.h"
#include "thread.h"
#include "func.h"



/* set_gdt --- 指定した GDT のエントリに値をセットする。
 *
 * 引数：
 *	int	index	新しく値をセットするGDTのエントリを指すエントリ
 *	VP	desc	ディスクリプタへのポインタ
 *			(ディスクリプタ自体は、8バイトの大きさをもつ)
 *
 * 返り値：
 *	エラー番号
 *		E_OK	正常時	
 *		E_PAR	indexの値がおかしい(GDTの範囲をオーバーしている)
 *
 */
ER
set_gdt (int index, GEN_DESC *desc)
{
  GEN_DESC	*p;

  if ((index <= 0) || (index > MAX_GDT))
    {
      return (E_PAR);	/* パラメータがおかしい */
    }
  p = (GEN_DESC *)GDT_ADDR;
  memcpy(&p[index], desc, sizeof(GEN_DESC));
  return (E_OK);
}
