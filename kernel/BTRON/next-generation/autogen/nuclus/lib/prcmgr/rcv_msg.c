/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rcv_msg
** Description: ��å������μ���
*/

ER
rcv_msg (W t_mask,MESSAGE *msg,W msgsz,UW opt)
{
	return call_btron(BSYS_RCV_MSG, t_mask, *msg, msgsz, opt);
}
