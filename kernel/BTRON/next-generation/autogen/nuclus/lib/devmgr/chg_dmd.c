/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_dmd
** Description: �ǥХ����Υ��������⡼���ѹ�
*/

ER
chg_dmd (TC *dev,UW mode)
{
	return call_btron(BSYS_CHG_DMD, *dev, mode);
}
