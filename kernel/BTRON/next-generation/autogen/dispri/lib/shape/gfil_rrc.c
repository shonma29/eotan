/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_rrc
** Description: -
*/

ER
gfil_rrc (W gid,RECT r,W rh,W rv,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFIL_RRC, gid, r, rh, rv, *pat, angle, mode);
}
