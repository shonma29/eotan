/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fat
** Description: ファイルのアクセス属性変更
*/

ER
chg_fat (LINK *lnk,W attr)
{
	return call_btron(BSYS_CHG_FAT, *lnk, attr);
}
