/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ins_rec
** Description: �쥳���ɤ�����
*/

ER
ins_rec (W fd,B *buf,LONG size,W type,UW subtype,UW units)
{
	return call_btron(BSYS_INS_REC, fd, *buf, size, type, subtype, units);
}
