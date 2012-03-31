/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_prc
** Description: プロセスの生成
*/

ER
cre_prc (LINK *lnk,W pri,MESSAGE *msg)
{
	return call_btron(BSYS_CRE_PRC, *lnk, pri, *msg);
}
