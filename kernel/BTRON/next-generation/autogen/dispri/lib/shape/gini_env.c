/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gini_env
** Description: -
*/

ER
gini_env (W gid)
{
	return call_btron(BSYS_GINI_ENV, gid);
}
