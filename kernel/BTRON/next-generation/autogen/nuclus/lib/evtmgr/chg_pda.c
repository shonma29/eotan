/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_pda
** Description: �ݥ���ƥ��󥰥ǥХ���°�����ѹ�
*/

ER
chg_pda (W attr)
{
	return call_btron(BSYS_CHG_PDA, attr);
}
