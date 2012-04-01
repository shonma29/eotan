/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsav_bmp
** Description: -
*/

ER
gsav_bmp (W gid,RPTR gr,BMPTR bmap,RPTR br,GSCAN *mask,W mode)
{
	return call_btron(BSYS_GSAV_BMP, gid, gr, bmap, br, *mask, mode);
}
