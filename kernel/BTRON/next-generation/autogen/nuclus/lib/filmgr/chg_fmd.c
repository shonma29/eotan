/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fmd
** Description: �ե�����Υ��������⡼���ѹ�
*/

ER
chg_fmd (LINK *lnk,A_MODE *mode)
{
	return call_btron(BSYS_CHG_FMD, *lnk, *mode);
}
