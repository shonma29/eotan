/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ktb
** Description: ʸ���������Ѵ�ɽ�μ��Ф�
*/

ER
get_ktb (KEYTAB *keytab)
{
	return call_btron(BSYS_GET_KTB, *keytab);
}
