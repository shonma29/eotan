/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_ptr
** Description: -
*/

ER
gmov_ptr (PNT pos)
{
	return call_btron(BSYS_GMOV_PTR, pos);
}
