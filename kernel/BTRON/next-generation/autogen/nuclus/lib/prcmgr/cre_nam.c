/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_nam
** Description: �����Х�̾�ǡ���������
*/

ER
cre_nam (TC *name,LONG data,UW opt)
{
	return call_btron(BSYS_CRE_NAM, *name, data, opt);
}
