/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_fls
** Description: �ե����륷���ƥ�Υꥹ�ȥ��å�
*/

ER
lst_fls (F_ATTACH *buff,W cnt)
{
	return call_btron(BSYS_LST_FLS, *buff, cnt);
}
