/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_sts
** Description: ��ͭ����֥�å����֤μ��Ф�
*/

ER
smb_sts (W akey,SM_STATE *buff)
{
	return call_btron(BSYS_SMB_STS, akey, *buff);
}
