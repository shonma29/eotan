/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ver
** Description: OS�С���������μ��Ф�
*/

ER
get_ver (T_VER *version)
{
	return call_btron(BSYS_GET_VER, *version);
}
