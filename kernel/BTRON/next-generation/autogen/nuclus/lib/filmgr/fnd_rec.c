/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fnd_rec
** Description: �쥳���ɤθ���
*/

ER
fnd_rec (W fd,W mode,ULONG typemask,UW subtype,LPTR recnum)
{
	return call_btron(BSYS_FND_REC, fd, mode, typemask, subtype, recnum);
}
