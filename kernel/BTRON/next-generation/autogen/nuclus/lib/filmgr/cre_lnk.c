/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_lnk
** Description: リンクファイルの生成
*/

ER
cre_lnk (LINK *lnk,F_LINK *ref,W opt)
{
	return call_btron(BSYS_CRE_LNK, *lnk, *ref, opt);
}
