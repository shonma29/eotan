/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: msel_gmn
** Description: -
*/

ER
msel_gmn (W mid,PNT pos)
{
	return call_btron(BSYS_MSEL_GMN, mid, pos);
}
