/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gscr_rec
** Description: -
*/

ER
gscr_rec (W gid,RECT r,W dh,dv,RLPTR rlp,PAT *pat)
{
	return call_btron(BSYS_GSCR_REC, gid, r, dh, dv, rlp, *pat);
}
