/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mcre_gmn
** Description: -
*/

ER
mcre_gmn (GMENU *gm)
{
	return call_btron(BSYS_MCRE_GMN, *gm);
}
