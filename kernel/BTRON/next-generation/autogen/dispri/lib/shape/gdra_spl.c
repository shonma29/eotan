/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_spl
** Description: -
*/

ER
gdra_spl (W gid,W np,PPTR pt,UW attr,PAT *pat,W mode)
{
	return call_btron(BSYS_GDRA_SPL, gid, np, pt, attr, *pat, mode);
}
