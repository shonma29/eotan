/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: see_rec
** Description: �쥳���ɰ��֤ΰ�ư
*/

ER
see_rec (W fd,LONG offset,W mode,LPTR recnum)
{
	return call_btron(BSYS_SEE_REC, fd, offset, mode, recnum);
}
