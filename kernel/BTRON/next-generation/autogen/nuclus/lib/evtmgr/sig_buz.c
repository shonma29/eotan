/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: sig_buz
** Description: �֥�����ȯ��
*/

ER
sig_buz (UW type)
{
	return call_btron(BSYS_SIG_BUZ, type);
}
