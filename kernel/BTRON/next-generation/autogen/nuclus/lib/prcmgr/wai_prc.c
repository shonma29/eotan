/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wai_prc
** Description: �ץ����λ����Ԥ�
*/

ER
wai_prc (LONG time)
{
	return call_btron(BSYS_WAI_PRC, time);
}
