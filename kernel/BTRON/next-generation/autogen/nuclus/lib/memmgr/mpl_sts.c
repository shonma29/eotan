/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mpl_sts
** Description: ��ͭ����ס�����֤μ��Ф�
*/

ER
mpl_sts (W mpl_id,MP_STATE *buff)
{
	return call_btron(BSYS_MPL_STS, mpl_id, *buff);
}
