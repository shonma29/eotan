/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_krp
** Description: ������ԡ��ȴֳ֤�����
*/

ER
set_krp (W offset,W interval)
{
	return call_btron(BSYS_SET_KRP, offset, interval);
}
