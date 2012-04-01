/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_dbg
** Description: デバッグ対象プロセスの生成
*/

ER
cre_dbg (LINK *lnk,W pri,MESSAGE arg,W dpid)
{
	return call_btron(BSYS_CRE_DBG, *lnk, pri, arg, dpid);
}
