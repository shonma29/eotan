/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_spl
** Description: -
*/

ER
gpic_spl (PNT p,W np,PPTR pt,W width)
{
	return call_btron(BSYS_GPIC_SPL, p, np, pt, width);
}
