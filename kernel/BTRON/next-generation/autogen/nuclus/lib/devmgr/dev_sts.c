/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: dev_sts
** Description: �ǥХ����δ�������μ��Ф�
*/

ER
dev_sts (TC *dev,DEV_STATE *buf)
{
	return call_btron(BSYS_DEV_STS, *dev, *buf);
}
