/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fls
** Description: ファイルシステム情報の変更
*/

ER
chg_fls (TC *dev,TC *fs_name,TC *fs_locate)
{
	return call_btron(BSYS_CHG_FLS, *dev, *fs_name, *fs_locate);
}
