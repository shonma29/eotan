/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_fil
** Description: ファイルの削除
*/

ER
del_fil (LINK *org,LINK *lnk,W force)
{
	return call_btron(BSYS_DEL_FIL, *org, *lnk, force);
}
