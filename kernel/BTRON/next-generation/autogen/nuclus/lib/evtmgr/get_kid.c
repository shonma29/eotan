/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_kid
** Description: �����ܡ���ID�μ��Ф�
*/

ER
get_kid (KBD_ID *id)
{
	return call_btron(BSYS_GET_KID, *id);
}
