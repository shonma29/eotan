/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chw
** Description: -
*/

ER
gget_chw (W gid,TCODE ch)
{
	return call_btron(BSYS_GGET_CHW, gid, ch);
}
