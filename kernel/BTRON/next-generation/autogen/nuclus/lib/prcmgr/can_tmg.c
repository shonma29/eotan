/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: can_tmg
** Description: �����ॢ���ȥ�å������μ��ä�
*/

ER
can_tmg (void)
{
	return call_btron(BSYS_CAN_TMG);
}
