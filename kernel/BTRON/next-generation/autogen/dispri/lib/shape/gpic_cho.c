/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_cho
** Description: -
*/

ER
gpic_cho (PNT pt,RECT r,PNT sp,PNT ep,W angle,W width)
{
	return call_btron(BSYS_GPIC_CHO, pt, r, sp, ep, angle, width);
}
