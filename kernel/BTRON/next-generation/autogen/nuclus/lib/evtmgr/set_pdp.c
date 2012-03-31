/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_pdp
** Description: PD位置の設定
*/

ER
set_pdp (PNT pos)
{
	return call_btron(BSYS_SET_PDP, pos);
}
