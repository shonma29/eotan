/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_dtm
** Description: -
*/

ER
mchg_dtm (W time)
{
	return call_btron(BSYS_MCHG_DTM, time);
}
