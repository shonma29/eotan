/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rea_dev
** Description: �ǥХ�������Υǡ����ɤ߹���
*/

ER
rea_dev (W dd,LONG start,B *buf,LONG size,LPTR a_size,W *error)
{
	return call_btron(BSYS_REA_DEV, dd, start, *buf, size, a_size, *error);
}
