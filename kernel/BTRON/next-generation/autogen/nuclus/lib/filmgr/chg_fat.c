/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fat
** Description: �ե�����Υ�������°���ѹ�
*/

ER
chg_fat (LINK *lnk,W attr)
{
	return call_btron(BSYS_CHG_FAT, *lnk, attr);
}
