/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_etm
** Description: ���٥�ȥ����ޡ��μ��Ф�
*/

ER
get_etm (ULPTR time)
{
	return call_btron(BSYS_GET_ETM, time);
}
