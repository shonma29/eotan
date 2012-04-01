/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chp
** Description: -
*/

ER
gget_chp (W gid,W *h,W *v)
{
	return call_btron(BSYS_GGET_CHP, gid, *h, *v);
}
