/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_for
** Description: -
*/

ER
gget_for (W gid,RLPTR rlp)
{
	return call_btron(BSYS_GGET_FOR, gid, rlp);
}
