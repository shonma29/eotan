/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: trc_rec
** Description: �쥳���ɤΥ������̾�
*/

ER
trc_rec (W fd,LONG size)
{
	return call_btron(BSYS_TRC_REC, fd, size);
}
