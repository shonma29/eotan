/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_siz
** Description: ���������֥�å����������μ��Ф�
*/

ER
lmb_siz (B *mptr,LPTR size)
{
	return call_btron(BSYS_LMB_SIZ, *mptr, size);
}
