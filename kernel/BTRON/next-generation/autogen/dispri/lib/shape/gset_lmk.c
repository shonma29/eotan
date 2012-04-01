/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_lmk
** Description: -
*/

ER
gset_lmk (W gid,W nbytes,B *mask)
{
	return call_btron(BSYS_GSET_LMK, gid, nbytes, *mask);
}
