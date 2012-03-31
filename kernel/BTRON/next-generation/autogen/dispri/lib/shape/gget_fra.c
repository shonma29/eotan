/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_fra
** Description: -
*/

ER
gget_fra (W gid,RPTR rp)
{
	return call_btron(BSYS_GGET_FRA, gid, rp);
}
