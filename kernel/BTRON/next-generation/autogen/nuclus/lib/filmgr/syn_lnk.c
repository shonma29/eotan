/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: syn_lnk
** Description: ��󥯥ե������Ʊ��
*/

ER
syn_lnk (LINK *lnk,W opt)
{
	return call_btron(BSYS_SYN_LNK, *lnk, opt);
}
