/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_nam
** Description: �����Х�̾�ǡ����κ��
*/

ER
del_nam (TC *name)
{
	return call_btron(BSYS_DEL_NAM, *name);
}
