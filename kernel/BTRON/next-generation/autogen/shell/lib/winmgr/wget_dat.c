/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_dat
** Description: -
*/

ER
wget_dat (W wid,LPTR dat)
{
	return call_btron(BSYS_WGET_DAT, wid, dat);
}
