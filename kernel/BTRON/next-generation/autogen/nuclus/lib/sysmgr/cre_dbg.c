/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_dbg
** Description: �ǥХå��оݥץ���������
*/

ER
cre_dbg (LINK *lnk,W pri,MESSAGE arg,W dpid)
{
	return call_btron(BSYS_CRE_DBG, *lnk, pri, arg, dpid);
}
