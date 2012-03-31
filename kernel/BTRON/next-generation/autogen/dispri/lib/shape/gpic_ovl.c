/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_ovl
** Description: -
*/

ER
gpic_ovl (PNT pt,RECT r,W angle,W width)
{
	return call_btron(BSYS_GPIC_OVL, pt, r, angle, width);
}
