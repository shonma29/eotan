/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wopn_iwd
** Description: -
*/

ER
wopn_iwd (W gid)
{
	return call_btron(BSYS_WOPN_IWD, gid);
}
