/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chh
** Description: -
*/

ER
gget_chh (W gid,TCODE ch)
{
	return call_btron(BSYS_GGET_CHH, gid, ch);
}
