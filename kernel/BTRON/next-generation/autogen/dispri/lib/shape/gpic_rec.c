/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
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
