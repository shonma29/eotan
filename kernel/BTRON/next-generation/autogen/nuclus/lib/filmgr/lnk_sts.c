/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lnk_sts
** Description: ��󥯥ե��������μ��Ф�
*/

ER
lnk_sts (LINK *lnk,F_LINK *stat)
{
	return call_btron(BSYS_LNK_STS, *lnk, *stat);
}
