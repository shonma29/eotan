/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_arc
** Description: -
*/

ER
gdra_arc (W gid,RECT r,PNT sp,PNT ep,UW attr,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GDRA_ARC, gid, r, sp, ep, attr, *pat, angle, mode);
}
