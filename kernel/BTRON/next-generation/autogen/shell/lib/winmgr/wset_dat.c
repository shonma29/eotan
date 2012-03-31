/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_dat
** Description: -
*/

ER
wset_dat (W wid,LONG dat)
{
	return call_btron(BSYS_WSET_DAT, wid, dat);
}
