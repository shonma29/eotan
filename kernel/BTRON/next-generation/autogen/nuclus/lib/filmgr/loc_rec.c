/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: loc_rec
** Description: �쥳���ɤΥ�å�
*/

ER
loc_rec (W fd,W mode)
{
	return call_btron(BSYS_LOC_REC, fd, mode);
}
