/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wmov_drg
** Description: -
*/

ER
wmov_drg (WEVENT *evt,RPTR new)
{
	return call_btron(BSYS_WMOV_DRG, *evt, new);
}
