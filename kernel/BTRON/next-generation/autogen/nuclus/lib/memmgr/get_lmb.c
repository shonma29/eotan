/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_lmb
** Description: ���������֥�å��γ���
*/

ER
get_lmb (B **mptr,LONG size,UW opt)
{
	return call_btron(BSYS_GET_LMB, **mptr, size, opt);
}
