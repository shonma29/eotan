/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchg_dsp
** Description: -
*/

ER
wchg_dsp (WDDISP *atr,PAT *bgpat)
{
	return call_btron(BSYS_WCHG_DSP, *atr, *bgpat);
}
