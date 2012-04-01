/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wfnd_wnd
** Description: -
*/

ER
wfnd_wnd (PPTR gpos,PPTR lpos,W *wid)
{
	return call_btron(BSYS_WFND_WND, gpos, lpos, *wid);
}
