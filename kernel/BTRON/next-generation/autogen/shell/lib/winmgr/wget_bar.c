/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_bar
** Description: -
*/

ER
wget_bar (W wid,W *rbar,W *bbar,W *lbar)
{
	return call_btron(BSYS_WGET_BAR, wid, *rbar, *bbar, *lbar);
}
