/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_pln
** Description: -
*/

ER
gpic_pln (PNT pt,POLYGON *poly,W width)
{
	return call_btron(BSYS_GPIC_PLN, pt, *poly, width);
}
