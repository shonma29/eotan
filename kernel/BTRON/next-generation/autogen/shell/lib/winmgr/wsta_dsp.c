/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsta_dsp
** Description: -
*/

ER
wsta_dsp (W wid,RPTR r,RLPTR rlst)
{
	return call_btron(BSYS_WSTA_DSP, wid, r, rlst);
}
