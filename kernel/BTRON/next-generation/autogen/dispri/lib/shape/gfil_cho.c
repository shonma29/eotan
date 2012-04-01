/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_cho
** Description: -
*/

ER
gfil_cho (W gid,RECT r,PNT sp,PNT ep,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFIL_CHO, gid, r, sp, ep, *pat, angle, mode);
}
