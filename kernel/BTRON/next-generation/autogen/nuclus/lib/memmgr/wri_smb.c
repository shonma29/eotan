/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wri_smb
** Description: ��ͭ����֥�å��ν񤭹���
*/

ER
wri_smb (W akey,B *buff,LONG offset,LONG size,LPTR a_size)
{
	return call_btron(BSYS_WRI_SMB, akey, *buff, offset, size, a_size);
}
