/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: loc_prc
** Description: �ץ����Υ�å�/�����å�
*/

ER
loc_prc (W onoff)
{
	return call_btron(BSYS_LOC_PRC, onoff);
}
