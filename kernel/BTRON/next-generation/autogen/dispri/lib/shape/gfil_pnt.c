/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_pnt
** Description: -
*/

ER
gfil_pnt (W gid,PNT p,PAT *pat,W mode)
{
	return call_btron(BSYS_GFIL_PNT, gid, p, *pat, mode);
}
