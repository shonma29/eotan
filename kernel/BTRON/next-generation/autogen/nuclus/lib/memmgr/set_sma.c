/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_sma
** Description: ��ͭ����֥�å�°��������
*/

ER
set_sma (W akey,UW attr)
{
	return call_btron(BSYS_SET_SMA, akey, attr);
}
