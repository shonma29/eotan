/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: grst_bmp
** Description: -
*/

ER
grst_bmp (W gid,RPTR gr,BMPTR bmap,RPTR br,GSCAN *mask,W mode)
{
	return call_btron(BSYS_GRST_BMP, gid, gr, bmap, br, *mask, mode);
}
