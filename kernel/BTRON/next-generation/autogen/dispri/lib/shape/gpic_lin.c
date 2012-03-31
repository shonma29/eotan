/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_lin
** Description: -
*/

ER
gpic_lin (PNT pt,PNT p0,PNT p1,W width)
{
	return call_btron(BSYS_GPIC_LIN, pt, p0, p1, width);
}
