/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wlst_wnd
** Description: -
*/

ER
wlst_wnd (W wid,W size,W *wids)
{
	return call_btron(BSYS_WLST_WND, wid, size, *wids);
}
