/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_cor
** Description: -
*/

ER
gmov_cor (W gid,W dh,W dv)
{
	return call_btron(BSYS_GMOV_COR, gid, dh, dv);
}
