/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_fls
** Description: �ե����륷���ƥ������
*/

ER
del_fls (TC *dev,W eject)
{
	return call_btron(BSYS_DEL_FLS, *dev, eject);
}
