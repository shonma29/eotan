/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_bgp
** Description: -
*/

ER
wget_bgp (W wid,PAT *pat,W size)
{
	return call_btron(BSYS_WGET_BGP, wid, *pat, size);
}
