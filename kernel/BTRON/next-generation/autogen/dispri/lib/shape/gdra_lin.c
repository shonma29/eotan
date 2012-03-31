/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_lin
** Description: -
*/

ER
gdra_lin (W gid,PNT p1,PNT p2,UW attr,PAT *pat,W mode)
{
	return call_btron(BSYS_GDRA_LIN, gid, p1, p2, attr, *pat, mode);
}
