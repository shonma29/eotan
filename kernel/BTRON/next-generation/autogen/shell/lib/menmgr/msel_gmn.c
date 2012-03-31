/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: msel_gmn
** Description: -
*/

ER
msel_gmn (W mid,PNT pos)
{
	return call_btron(BSYS_MSEL_GMN, mid, pos);
}
