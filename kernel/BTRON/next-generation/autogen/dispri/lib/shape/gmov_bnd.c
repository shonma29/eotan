/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_bnd
** Description: -
*/

ER
gmov_bnd (W gid,W dh,W dv)
{
	return call_btron(BSYS_GMOV_BND, gid, dh, dv);
}
