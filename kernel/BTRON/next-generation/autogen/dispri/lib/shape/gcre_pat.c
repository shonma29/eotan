/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcre_pat
** Description: -
*/

ER
gcre_pat (W gid,PAT *def,B *loc,B **pat,LPTR size)
{
	return call_btron(BSYS_GCRE_PAT, gid, *def, *loc, **pat, size);
}
