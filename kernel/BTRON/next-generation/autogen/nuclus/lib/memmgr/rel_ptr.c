/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_ptr
** Description: ����ݥ��󥿤γ���
*/

ER
rel_ptr (B *mptr)
{
	return call_btron(BSYS_REL_PTR, *mptr);
}
