/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_rec
** Description: �쥳���ɤκ��
*/

ER
del_rec (W fd)
{
	return call_btron(BSYS_DEL_REC, fd);
}
