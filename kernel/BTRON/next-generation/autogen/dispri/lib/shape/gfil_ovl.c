/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_ovl
** Description: -
*/

ER
gfil_ovl (W gid,RECT r,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFIL_OVL, gid, r, *pat, angle, mode);
}
