/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mset_itm
** Description: -
*/

ER
mset_itm (W mid,W pnum,MENUITEM *item)
{
	return call_btron(BSYS_MSET_ITM, mid, pnum, *item);
}
