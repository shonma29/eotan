/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_dmd
** Description: デバイスのアクセスモード変更
*/

ER
chg_dmd (TC *dev,UW mode)
{
	return call_btron(BSYS_CHG_DMD, *dev, mode);
}
