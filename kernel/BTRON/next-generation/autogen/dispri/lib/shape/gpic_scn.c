/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_scn
** Description: -
*/

ER
gpic_scn (PNT pt,GSCAN *scan)
{
	return call_btron(BSYS_GPIC_SCN, pt, *scan);
}
