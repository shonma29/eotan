/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_cup
** Description: クリーンアップハンドラの登録
*/

ER
def_cup (FUNCP clup_hdr)
{
	return call_btron(BSYS_DEF_CUP, clup_hdr);
}
