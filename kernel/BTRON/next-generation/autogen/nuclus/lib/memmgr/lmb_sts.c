/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_sts
** Description: ����������ΰ�ξ��ּ��Ф�
*/

ER
lmb_sts (LM_STATE *buff)
{
	return call_btron(BSYS_LMB_STS, *buff);
}
