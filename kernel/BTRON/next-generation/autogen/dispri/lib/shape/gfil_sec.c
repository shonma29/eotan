/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_sec
** Description: -
*/

ER
gfil_sec (W gid,RECT r,PNT sp,PNT ep,PAT *pat,W angle,W mode)
{
	return call_btron(BSYS_GFIL_SEC, gid, r, sp, ep, *pat, angle, mode);
}
