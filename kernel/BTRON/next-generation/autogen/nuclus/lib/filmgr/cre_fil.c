/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_fil
** Description: �ե����������
*/

ER
cre_fil (LINK *lnk,TC *name,A_MODE *mode,UW atype,W opt)
{
	return call_btron(BSYS_CRE_FIL, *lnk, *name, *mode, atype, opt);
}
