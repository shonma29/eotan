/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_col
** Description: -
*/

ER
gset_col (W gid,W p,COLORVAL *cv,W cnt)
{
	return call_btron(BSYS_GSET_COL, gid, p, *cv, cnt);
}
