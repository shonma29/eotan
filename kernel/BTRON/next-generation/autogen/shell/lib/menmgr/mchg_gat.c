/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_gat
** Description: -
*/

ER
mchg_gat (W mid,W num,UW mode)
{
	return call_btron(BSYS_MCHG_GAT, mid, num, mode);
}
