/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fil_sts
** Description: �ե��������μ��Ф�
*/

ER
fil_sts (LINK *lnk,TC *name,F_STATE *stat,F_LOCATE *locat)
{
	return call_btron(BSYS_FIL_STS, *lnk, *name, *stat, *locat);
}
