/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcnv_col
** Description: -
*/

ER
gcnv_col (W gid,COLORVAL cv,LPTR pixv)
{
	return call_btron(BSYS_GCNV_COL, gid, cv, pixv);
}
