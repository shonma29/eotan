/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lnk_sts
** Description: リンクファイル情報の取り出し
*/

ER
lnk_sts (LINK *lnk,F_LINK *stat)
{
	return call_btron(BSYS_LNK_STS, *lnk, *stat);
}
