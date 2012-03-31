/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_sts
** Description: -
*/

ER
wget_sts (W wid,WDSTAT *stat,WDDISP *atr)
{
	return call_btron(BSYS_WGET_STS, wid, *stat, *atr);
}
