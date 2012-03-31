/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsiz_cbm
** Description: -
*/

ER
gsiz_cbm (W gid,RPTR rp,W compac,LPTR size)
{
	return call_btron(BSYS_GSIZ_CBM, gid, rp, compac, size);
}
