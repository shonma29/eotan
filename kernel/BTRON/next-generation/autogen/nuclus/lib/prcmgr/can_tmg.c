/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: can_tmg
** Description: タイムアウトメッセージの取り消し
*/

ER
can_tmg (void)
{
	return call_btron(BSYS_CAN_TMG);
}
