/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_dev
** Description: �����ǥХ���̾�μ��Ф�
*/

ER
get_dev (TC *dev,W num)
{
	return call_btron(BSYS_GET_DEV, *dev, num);
}
