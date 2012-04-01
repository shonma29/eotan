/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_pol
** Description: -
*/

ER
gfil_pol (W gid,POLYGON *p,PAT *pat,W mode)
{
	return call_btron(BSYS_GFIL_POL, gid, *p, *pat, mode);
}
