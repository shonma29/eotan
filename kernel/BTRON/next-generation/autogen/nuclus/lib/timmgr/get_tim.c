/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_tim
** Description: �����ƥ���֤μ��Ф�
*/

ER
get_tim (LPTR time,TIMEZONE *tz)
{
	return call_btron(BSYS_GET_TIM, time, *tz);
}
