/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: clr_evt
** Description: ���٥�Ȥκ��
*/

ER
clr_evt (W t_mask,W lask_mask)
{
	return call_btron(BSYS_CLR_EVT, t_mask, lask_mask);
}
