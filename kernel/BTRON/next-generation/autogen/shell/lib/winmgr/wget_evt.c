/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_evt
** Description: -
*/

ER
wget_evt (WEVENT *evt,W mode)
{
	return call_btron(BSYS_WGET_EVT, *evt, mode);
}
