/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wcls_wnd
** Description: -
*/

ER
wcls_wnd (W wid,W opt)
{
	return call_btron(BSYS_WCLS_WND, wid, opt);
}
