/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_nam
** Description: グローバル名データの生成
*/

ER
cre_nam (TC *name,LONG data,UW opt)
{
	return call_btron(BSYS_CRE_NAM, *name, data, opt);
}
