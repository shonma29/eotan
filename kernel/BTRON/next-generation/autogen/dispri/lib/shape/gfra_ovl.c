/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_ovl
** Description: -
*/

ER
gfra_ovl (W gid,RECT r,UW attr,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFRA_OVL, gid, r, attr, *pat, angle, mode);
}
