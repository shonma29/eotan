/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsnd_evt
** Description: -
*/

ER
wsnd_evt (WEVENT *evt)
{
	return call_btron(BSYS_WSND_EVT, *evt);
}
