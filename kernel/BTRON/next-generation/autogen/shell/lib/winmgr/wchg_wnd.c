/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchg_wnd
** Description: -
*/

ER
wchg_wnd (W wid,RPTR r,W mode)
{
	return call_btron(BSYS_WCHG_WND, wid, r, mode);
}
