/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wopn_pwd
** Description: -
*/

ER
wopn_pwd (RPTR r)
{
	return call_btron(BSYS_WOPN_PWD, r);
}
