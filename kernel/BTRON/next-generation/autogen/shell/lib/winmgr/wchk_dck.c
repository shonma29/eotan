/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchk_dck
** Description: -
*/

ER
wchk_dck (ULONG first)
{
	return call_btron(BSYS_WCHK_DCK, first);
}
