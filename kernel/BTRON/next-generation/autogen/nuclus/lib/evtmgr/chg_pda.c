/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_pda
** Description: ポインティングデバイス属性の変更
*/

ER
chg_pda (W attr)
{
	return call_btron(BSYS_CHG_PDA, attr);
}
