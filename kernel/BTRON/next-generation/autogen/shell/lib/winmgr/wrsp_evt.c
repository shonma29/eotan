/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wrsp_evt
** Description: -
*/

ER
wrsp_evt (WEVENT *evt,W nak)
{
	return call_btron(BSYS_WRSP_EVT, *evt, nak);
}
