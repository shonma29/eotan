/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: prc_trc
** Description: �ǥХå��оݥץ���������
*/

ER
prc_trc (TRACE trace)
{
	return call_btron(BSYS_PRC_TRC, trace);
}
