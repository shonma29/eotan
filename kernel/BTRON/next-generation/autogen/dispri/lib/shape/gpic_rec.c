/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_rec
** Description: -
*/

ER
gpic_rec (PNT pt,RECT r,W angle,W width)
{
	return call_btron(BSYS_GPIC_REC, pt, r, angle, width);
}
