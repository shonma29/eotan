/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdsp_ptr
** Description: -
*/

ER
gdsp_ptr (W req)
{
	return call_btron(BSYS_GDSP_PTR, req);
}
