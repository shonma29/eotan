/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_dfm
** Description: �ǥե���ȡ����������⡼�ɤμ��Ф�
*/

ER
get_dfm (DA_MODE *mode)
{
	return call_btron(BSYS_GET_DFM, *mode);
}
