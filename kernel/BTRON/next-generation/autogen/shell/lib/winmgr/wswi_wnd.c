/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wswi_wnd
** Description: -
*/

ER
wswi_wnd (W wid,WEVENT *evt)
{
	return call_btron(BSYS_WSWI_WND, wid, *evt);
}
