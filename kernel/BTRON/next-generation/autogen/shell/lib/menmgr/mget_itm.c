/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mget_itm
** Description: -
*/

ER
mget_itm (W mid,W pnum,MENUITEM *item)
{
	return call_btron(BSYS_MGET_ITM, mid, pnum, *item);
}
