/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wend_dsp
** Description: -
*/

ER
wend_dsp (W wid)
{
	return call_btron(BSYS_WEND_DSP, wid);
}
