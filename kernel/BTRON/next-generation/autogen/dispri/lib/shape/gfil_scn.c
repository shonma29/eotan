/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_scn
** Description: -
*/

ER
gfil_scn (W gid,GSCAN *scan,PAT *pat,W mode)
{
	return call_btron(BSYS_GFIL_SCN, gid, *scan, *pat, mode);
}
