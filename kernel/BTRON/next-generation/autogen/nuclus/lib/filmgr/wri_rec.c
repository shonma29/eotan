/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wri_rec
** Description: �쥳���ɤν񤭹���
*/

ER
wri_rec (W fd,LONG offset,B *buf,LONG size,LPTR r_size,UW *subtype,UW units)
{
	return call_btron(BSYS_WRI_REC, fd, offset, *buf, size, r_size, *subtype, units);
}
