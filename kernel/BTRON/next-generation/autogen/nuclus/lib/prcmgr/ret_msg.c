/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_msg
** Description: ��å������ϥ�ɥ�ν�λ
*/

ER
ret_msg (W ret)
{
	return call_btron(BSYS_RET_MSG, ret);
}
