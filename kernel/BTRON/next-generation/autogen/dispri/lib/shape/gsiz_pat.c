/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsiz_pat
** Description: -
*/

ER
gsiz_pat (W gid,W kind,W hsize,W vsize,LPTR size)
{
	return call_btron(BSYS_GSIZ_PAT, gid, kind, hsize, vsize, size);
}
