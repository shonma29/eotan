/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_smi
** Description: ��ͭ����֥�å��Υ桼�����������
*/

ER
set_smi (W akey,UW attr)
{
	return call_btron(BSYS_SET_SMI, akey, attr);
}
