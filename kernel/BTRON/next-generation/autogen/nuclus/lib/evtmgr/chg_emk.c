/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_emk
** Description: �����ƥࡦ���٥�ȥޥ������ѹ�
*/

ER
chg_emk (W mask)
{
	return call_btron(BSYS_CHG_EMK, mask);
}
