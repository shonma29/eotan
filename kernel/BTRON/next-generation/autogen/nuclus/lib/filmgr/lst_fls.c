/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_fls
** Description: ファイルシステムのリストアップ
*/

ER
lst_fls (F_ATTACH *buff,W cnt)
{
	return call_btron(BSYS_LST_FLS, *buff, cnt);
}
