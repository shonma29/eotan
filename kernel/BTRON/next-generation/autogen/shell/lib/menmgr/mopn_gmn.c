/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mopn_gmn
** Description: -
*/

ER
mopn_gmn (W dnum)
{
	return call_btron(BSYS_MOPN_GMN, dnum);
}
