/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_dfm
** Description: �ǥե���ȡ����������⡼�ɤ�����
*/

ER
set_dfm (DA_MODE *mode)
{
	return call_btron(BSYS_SET_DFM, *mode);
}
