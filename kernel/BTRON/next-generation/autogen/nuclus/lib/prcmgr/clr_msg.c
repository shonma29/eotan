/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: clr_msg
** Description: ��å������Υ��ꥢ
*/

ER
clr_msg (LONG t_mask,LONG last_mask)
{
	return call_btron(BSYS_CLR_MSG, t_mask, last_mask);
}
