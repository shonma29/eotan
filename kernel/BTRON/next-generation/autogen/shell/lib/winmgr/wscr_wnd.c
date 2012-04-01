/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wscr_wnd
** Description: -
*/

ER
wscr_wnd (W wid,RPTR r,W dh,W dv,W mode)
{
	return call_btron(BSYS_WSCR_WND, wid, r, dh, dv, mode);
}
