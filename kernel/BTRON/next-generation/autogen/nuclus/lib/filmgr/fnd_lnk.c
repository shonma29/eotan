/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fnd_lnk
** Description: ��󥯥쥳���ɤθ���
*/

ER
fnd_lnk (W fd,W mode,LINK *lnk,UW subtype,LPTR recnum)
{
	return call_btron(BSYS_FND_LNK, fd, mode, *lnk, subtype, recnum);
}
