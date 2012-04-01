/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_rec
** Description: -
*/

ER
gfra_rec (W gid,RECT r,UW attr,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFRA_REC, gid, r, attr, *pat, angle, mode);
}
