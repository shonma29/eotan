/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: prc_sts
** Description: �ץ������֤μ��Ф�
*/

ER
prc_sts (W pid,P_STATE *buff,TC *path)
{
	return call_btron(BSYS_PRC_STS, pid, *buff, *path);
}
