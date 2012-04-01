/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_fil
** Description: ファイルの生成
*/

ER
cre_fil (LINK *lnk,TC *name,A_MODE *mode,UW atype,W opt)
{
	return call_btron(BSYS_CRE_FIL, *lnk, *name, *mode, atype, opt);
}
