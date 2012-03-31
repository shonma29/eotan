/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wdef_fep
** Description: -
*/

ER
wdef_fep (W onoff)
{
	return call_btron(BSYS_WDEF_FEP, onoff);
}
