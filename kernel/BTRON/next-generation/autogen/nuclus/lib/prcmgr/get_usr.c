/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_usr
** Description: �ץ����Υ桼������μ��Ф�
*/

ER
get_usr (W pid,P_USER *buff)
{
	return call_btron(BSYS_GET_USR, pid, *buff);
}
