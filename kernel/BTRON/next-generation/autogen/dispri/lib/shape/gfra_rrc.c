/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_rrc
** Description: -
*/

ER
gfra_rrc (W gid,RECT r,W rh,W rv,UW attr,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFRA_RRC, gid, r, rh, rv, attr, *pat, angle, mode);
}
