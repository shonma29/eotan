/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: syn_fls
** Description: �ե����륷���ƥ��Ʊ��
*/

ER
syn_fls (void)
{
	return call_btron(BSYS_SYN_FLS);
}
