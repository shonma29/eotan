/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_pol
** Description: -
*/

ER
gfra_pol (W gid,POLYGON *p,UW attr,PAT *pat,W mode)
{
	return call_btron(BSYS_GFRA_POL, gid, *p, attr, *pat, mode);
}
