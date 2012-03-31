/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_emk
** Description: システム・イベントマスクの変更
*/

ER
chg_emk (W mask)
{
	return call_btron(BSYS_CHG_EMK, mask);
}
