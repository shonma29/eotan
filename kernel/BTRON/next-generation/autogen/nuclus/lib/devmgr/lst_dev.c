/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_dev
** Description: ��Ͽ�ѤߥǥХ����μ��Ф�
*/

ER
lst_dev (DEV_INFO *dev,W ndev)
{
	return call_btron(BSYS_LST_DEV, *dev, ndev);
}
