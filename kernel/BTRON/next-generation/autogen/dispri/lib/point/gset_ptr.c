/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_ptr
** Description: -
*/

ER
gset_ptr (W style,PTRIMAGE *img,COLORVAL fgcol,COLORVAL bgcol)
{
	return call_btron(BSYS_GSET_PTR, style, *img, fgcol, bgcol);
}
