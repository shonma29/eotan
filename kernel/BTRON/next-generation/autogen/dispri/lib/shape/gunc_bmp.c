/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gunc_bmp
** Description: -
*/

ER
gunc_bmp (W gid,RPTR dr,C_BMPTR cbmp,RPTR sr,W mode)
{
	return call_btron(BSYS_GUNC_BMP, gid, dr, cbmp, sr, mode);
}
