/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchk_dsp
** Description: -
*/

ER
wchk_dsp (W wid)
{
	return call_btron(BSYS_WCHK_DSP, wid);
}
