/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rcr_prc
** Description: プロセスの再生成
*/

ER
rcr_prc (LINK *lnk,MESSAGE *msg)
{
	return call_btron(BSYS_RCR_PRC, *lnk, *msg);
}
