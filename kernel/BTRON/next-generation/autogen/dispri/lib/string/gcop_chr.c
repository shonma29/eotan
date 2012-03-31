/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcop_chr
** Description: -
*/

ER
gcop_chr (W gid,PNT asize,PNT isize,B *img,W mode)
{
	return call_btron(BSYS_GCOP_CHR, gid, asize, isize, *img, mode);
}
