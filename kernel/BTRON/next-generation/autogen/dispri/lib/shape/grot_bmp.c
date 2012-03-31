/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: grot_bmp
** Description: -
*/

ER
grot_bmp (W srcid,W dstid,RPTR rp,PPTR dp,W angle,GSCAN *mask,W mode)
{
	return call_btron(BSYS_GROT_BMP, srcid, dstid, rp, dp, angle, *mask, mode);
}
