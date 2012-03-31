/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: msel_men
** Description: -
*/

ER
msel_men (W mid,PNT pos)
{
	return call_btron(BSYS_MSEL_MEN, mid, pos);
}
