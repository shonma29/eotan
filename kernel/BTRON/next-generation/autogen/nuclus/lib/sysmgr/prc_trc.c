/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: prc_trc
** Description: デバッグ対象プロセスの制御
*/

ER
prc_trc (TRACE trace)
{
	return call_btron(BSYS_PRC_TRC, trace);
}
