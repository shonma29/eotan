/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_mpl
** Description: ����ס��������
*/

ER
cre_mpl (W mpl_id,LONG size,UW attr)
{
	return call_btron(BSYS_CRE_MPL, mpl_id, size, attr);
}
