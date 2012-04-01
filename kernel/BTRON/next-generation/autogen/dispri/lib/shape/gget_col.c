/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_col
** Description: -
*/

ER
gget_col (W gid,W p,COLORVAL *cv,W cnt)
{
	return call_btron(BSYS_GGET_COL, gid, p, *cv, cnt);
}
