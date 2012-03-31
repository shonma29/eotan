/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_ftm
** Description: ファイル日時の変更
*/

ER
chg_ftm (LINK *lnk,F_TIME *times)
{
	return call_btron(BSYS_CHG_FTM, *lnk, *times);
}
