/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gen_fil
** Description: ファイルの直接生成
*/

ER
gen_fil (LINK *lnk,F_LINK *ref,W opt)
{
	return call_btron(BSYS_GEN_FIL, *lnk, *ref, opt);
}
