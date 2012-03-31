/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_rgn
** Description: -
*/

ER
gfil_rgn (W gid,LONG val,PNT p,PAT *pat,W mode)
{
	return call_btron(BSYS_GFIL_RGN, gid, val, p, *pat, mode);
}
