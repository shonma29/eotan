/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mopn_gmn
** Description: -
*/

ER
mopn_gmn (W dnum)
{
	return call_btron(BSYS_MOPN_GMN, dnum);
}
