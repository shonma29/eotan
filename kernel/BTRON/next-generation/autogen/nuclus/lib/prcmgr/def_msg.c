/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_msg
** Description: ��å������ϥ�ɥ�����
*/

ER
def_msg (LONG t_mask,FUNCP msg_hdr)
{
	return call_btron(BSYS_DEF_MSG, t_mask, msg_hdr);
}
