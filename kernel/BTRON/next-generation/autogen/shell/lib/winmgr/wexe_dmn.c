/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wexe_dmn
** Description: -
*/

ER
wexe_dmn (W item)
{
	return call_btron(BSYS_WEXE_DMN, item);
}
