/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_exc
** Description: �㳰�����ϥ�ɥ�ν�λ
*/

ER
ret_exc (W ret)
{
	return call_btron(BSYS_RET_EXC, ret);
}
