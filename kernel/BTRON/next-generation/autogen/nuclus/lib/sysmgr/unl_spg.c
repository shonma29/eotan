/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: unl_spg
** Description: �����ƥ�ץ����Υ������
*/

ER
unl_spg (W prog_id)
{
	return call_btron(BSYS_UNL_SPG, prog_id);
}
