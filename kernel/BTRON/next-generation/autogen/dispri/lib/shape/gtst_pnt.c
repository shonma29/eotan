/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gtst_pnt
** Description: -
*/

ER
gtst_pnt (W gid,PNT p,LPTR val)
{
	return call_btron(BSYS_GTST_PNT, gid, p, val);
}
