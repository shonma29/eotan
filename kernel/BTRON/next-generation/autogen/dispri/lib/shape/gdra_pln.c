/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_pln
** Description: -
*/

ER
gdra_pln (W gid,POLYGON *p,UW attr,PAT *pat,W mode)
{
	return call_btron(BSYS_GDRA_PLN, gid, *p, attr, *pat, mode);
}
