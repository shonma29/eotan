/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cls_dev
** Description: �ǥХ����Υ�����
*/

ER
cls_dev (W dd,W eject,W *error)
{
	return call_btron(BSYS_CLS_DEV, dd, eject, *error);
}
