/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_pnt
** Description: -
*/

ER
gdra_pnt (W gid,PNT p,LONG val,W mode)
{
	return call_btron(BSYS_GDRA_PNT, gid, p, val, mode);
}
