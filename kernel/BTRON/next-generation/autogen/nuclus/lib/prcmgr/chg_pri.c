/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_pri
** Description: �ץ�����ͥ�����ѹ�
*/

ER
chg_pri (W pid,W new_pri,UW opt)
{
	return call_btron(BSYS_CHG_PRI, pid, new_pri, opt);
}
