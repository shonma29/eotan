/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_adr
** Description: ��ͭ����֥�å������ɥ쥹�μ��Ф�
*/

ER
smb_adr (W akey,B **maddr)
{
	return call_btron(BSYS_SMB_ADR, akey, **maddr);
}
