/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: apd_rec
** Description: �쥳���ɤ��ɲ�
*/

ER
apd_rec (W fd,B *buf,LONG size,W type,UW subtype,UW units)
{
	return call_btron(BSYS_APD_REC, fd, *buf, size, type, subtype, units);
}
