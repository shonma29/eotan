/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_rrc
** Description: -
*/

ER
gpic_rrc (PNT pt,RECT r,W rh,W rv,W angle,W width)
{
	return call_btron(BSYS_GPIC_RRC, pt, r, rh, rv, angle, width);
}
