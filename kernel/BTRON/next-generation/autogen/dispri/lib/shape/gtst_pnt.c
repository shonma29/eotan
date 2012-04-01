/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gtst_pnt
** Description: -
*/

ER
gtst_pnt (W gid,PNT p,LPTR val)
{
	return call_btron(BSYS_GTST_PNT, gid, p, val);
}
