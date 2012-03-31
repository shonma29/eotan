/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wrsp_evt
** Description: -
*/

ER
wrsp_evt (WEVENT *evt,W nak)
{
	return call_btron(BSYS_WRSP_EVT, *evt, nak);
}
