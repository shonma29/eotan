/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chk_fil
** Description: �ե�����Υ��������������å�
*/

ER
chk_fil (LINK *lnk,UW mode,TC *pwd)
{
	return call_btron(BSYS_CHK_FIL, *lnk, mode, *pwd);
}
