/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_ptr
** Description: -
*/

ER
gget_ptr (PTRSTS *sts,PTRIMAGE *img)
{
	return call_btron(BSYS_GGET_PTR, *sts, *img);
}
