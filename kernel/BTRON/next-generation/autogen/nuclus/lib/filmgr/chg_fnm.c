/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fnm
** Description: ファイル名の変更
*/

ER
chg_fnm (LINK *lnk,TC *name)
{
	return call_btron(BSYS_CHG_FNM, *lnk, *name);
}
