/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_wrk
** Description: 作業ファイルの変更
*/

ER
chg_wrk (LINK *lnk)
{
	return call_btron(BSYS_CHG_WRK, *lnk);
}
