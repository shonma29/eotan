/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmot_ptr
** Description: -
*/

ER
gmot_ptr (W sts)
{
	return call_btron(BSYS_GMOT_PTR, sts);
}
