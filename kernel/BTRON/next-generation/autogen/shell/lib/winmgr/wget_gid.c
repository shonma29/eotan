/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_gid
** Description: -
*/

ER
wget_gid (W wid)
{
	return call_btron(BSYS_WGET_GID, wid);
}
