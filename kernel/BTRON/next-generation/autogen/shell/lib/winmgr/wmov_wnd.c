/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wmov_wnd
** Description: -
*/

ER
wmov_wnd (W wid,RPTR new)
{
	return call_btron(BSYS_WMOV_WND, wid, new);
}
