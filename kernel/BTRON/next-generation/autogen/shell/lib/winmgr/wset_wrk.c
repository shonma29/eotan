/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_wrk
** Description: -
*/

ER
wset_wrk (W wid,RPTR r)
{
	return call_btron(BSYS_WSET_WRK, wid, r);
}
