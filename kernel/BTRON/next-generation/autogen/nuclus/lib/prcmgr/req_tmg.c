/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: req_tmg
** Description: �����ॢ���ȥ�å��������׵�
*/

ER
req_tmg (LONG time,W code)
{
	return call_btron(BSYS_REQ_TMG, time, code);
}
