/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_vis
** Description: -
*/

ER
gset_vis (W gid,RECT r)
{
	return call_btron(BSYS_GSET_VIS, gid, r);
}
