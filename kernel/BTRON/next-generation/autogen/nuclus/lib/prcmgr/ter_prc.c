/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ter_prc
** Description: ¾�ץ����ζ�����λ
*/

ER
ter_prc (W pid,W abort_code,UW opt)
{
	return call_btron(BSYS_TER_PRC, pid, abort_code, opt);
}
