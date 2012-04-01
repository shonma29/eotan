/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_exc
** Description: 例外処理ハンドラの定義
*/

ER
def_exc (W exc_vec,FUNCP exc_hdr)
{
	return call_btron(BSYS_DEF_EXC, exc_vec, exc_hdr);
}
