/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rcr_prc
** Description: �ץ����κ�����
*/

ER
rcr_prc (LINK *lnk,MESSAGE *msg)
{
	return call_btron(BSYS_RCR_PRC, *lnk, *msg);
}
