/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_prc
** Description: �ץ���������
*/

ER
cre_prc (LINK *lnk,W pri,MESSAGE *msg)
{
	return call_btron(BSYS_CRE_PRC, *lnk, pri, *msg);
}
