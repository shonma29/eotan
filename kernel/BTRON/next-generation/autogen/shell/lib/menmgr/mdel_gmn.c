/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mdel_gmn
** Description: -
*/

ER
mdel_gmn (W mid)
{
	return call_btron(BSYS_MDEL_GMN, mid);
}
