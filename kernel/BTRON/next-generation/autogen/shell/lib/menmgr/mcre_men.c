/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mcre_men
** Description: -
*/

ER
mcre_men (W nitem,MENUITEM *item,MENUDISP *attr)
{
	return call_btron(BSYS_MCRE_MEN, nitem, *item, *attr);
}
