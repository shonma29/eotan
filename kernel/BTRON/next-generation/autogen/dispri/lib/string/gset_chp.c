/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_chp
** Description: -
*/

ER
gset_chp (W gid,W h,W v,W abs)
{
	return call_btron(BSYS_GSET_CHP, gid, h, v, abs);
}
