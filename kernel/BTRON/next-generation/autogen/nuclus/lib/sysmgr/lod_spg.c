/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lod_spg
** Description: システムプログラムのロード
*/

ER
lod_spg (LINK *lnk,TC *arg,UW attr,LPTR info)
{
	return call_btron(BSYS_LOD_SPG, *lnk, *arg, attr, info);
}
