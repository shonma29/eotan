/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fls_sts
** Description: �ե����륷���ƥ�δ���������Ф�
*/

ER
fls_sts (TC *dev,FS_STATE *buff)
{
	return call_btron(BSYS_FLS_STS, *dev, *buff);
}
