/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_usr
** Description: �ץ����Υ桼��������ѹ�
*/

ER
chg_usr (P_USER *buff)
{
	return call_btron(BSYS_CHG_USR, *buff);
}
