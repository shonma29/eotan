/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wpas_evt
** Description: -
*/

ER
wpas_evt (WEVENT *evt)
{
	return call_btron(BSYS_WPAS_EVT, *evt);
}
