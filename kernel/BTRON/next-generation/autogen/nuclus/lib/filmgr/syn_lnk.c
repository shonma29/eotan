/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: syn_lnk
** Description: リンクファイルの同期
*/

ER
syn_lnk (LINK *lnk,W opt)
{
	return call_btron(BSYS_SYN_LNK, *lnk, opt);
}
