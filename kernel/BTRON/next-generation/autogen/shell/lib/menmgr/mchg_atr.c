/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_atr
** Description: -
*/

ER
mchg_atr (W mid,W selnum,UW mode)
{
	return call_btron(BSYS_MCHG_ATR, mid, selnum, mode);
}
