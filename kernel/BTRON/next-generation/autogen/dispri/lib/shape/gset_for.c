/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_for
** Description: -
*/

ER
gset_for (W gid,RLPTR rlp)
{
	return call_btron(BSYS_GSET_FOR, gid, rlp);
}
