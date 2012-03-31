/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: req_tmg
** Description: タイムアウトメッセージの要求
*/

ER
req_tmg (LONG time,W code)
{
	return call_btron(BSYS_REQ_TMG, time, code);
}
