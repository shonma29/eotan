/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: att_fls
** Description: �ե����륷���ƥ����³
*/

ER
att_fls (TC *dev,TC *name,LINK *lnk,W r_only)
{
	return call_btron(BSYS_ATT_FLS, *dev, *name, *lnk, r_only);
}
