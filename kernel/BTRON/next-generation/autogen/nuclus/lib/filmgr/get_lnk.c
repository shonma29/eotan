/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_lnk
** Description: �ե�����ؤΥ�󥯤μ��Ф�
*/

ER
get_lnk (TC *path,LINK *lnk,W mode)
{
	return call_btron(BSYS_GET_LNK, *path, *lnk, mode);
}
